#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include "output_sink.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

extern char** environ;

namespace tiecodec {
namespace test {
namespace {

namespace fs = std::filesystem;

constexpr const char* kCompilerEnvVar = "TIECODEC_TEST_COMPILER";

constexpr const char* kSmokeSource = R"(类 启动窗口 : 窗口
  变量 问候: 文本 = "你好"

  事件 启动窗口:创建完毕()
    调试输出(问候)
  结束 事件
结束 类
)";

bool EndsWith(const std::string& text, const std::string& suffix) {
  return text.size() >= suffix.size() &&
         text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string ReadFile(const fs::path& path) {
  std::ifstream input(path, std::ios::binary);
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

bool Contains(const std::string& text, const std::string& needle) {
  return text.find(needle) != std::string::npos;
}

std::vector<fs::path> CollectSources(const fs::path& directory) {
  std::vector<fs::path> sources;
  if (!fs::is_directory(directory))
    return sources;
  for (const fs::directory_entry& entry : fs::recursive_directory_iterator(directory)) {
    if (!entry.is_regular_file())
      continue;
    if (!EndsWith(entry.path().filename().string(), ".t"))
      continue;
    sources.push_back(entry.path());
  }
  std::sort(sources.begin(), sources.end());
  return sources;
}

struct ProcessResult {
  int status = -1;
  std::string log;
};

class SmokeTest : public ::testing::Test {
protected:
  void SetUp() override {
    compiler_ = CompilerPath();
    ASSERT_FALSE(compiler_.empty()) << "未提供编译器路径（环境变量 " << kCompilerEnvVar << "）";
    ASSERT_TRUE(fs::is_regular_file(compiler_)) << "缺少编译器: " << compiler_;

    stdlib_ = fs::path(TIECODEC_SOURCE_DIR) / "stdlibs" / "安卓基本库" / "源代码";
    ASSERT_TRUE(fs::is_directory(stdlib_)) << "缺少标准库源文件: " << stdlib_;

    workspace_ = fs::temp_directory_path() / UniqueDirectoryName();
    fs::create_directories(workspace_ / "src");
    std::ofstream out(workspace_ / "src" / "启动窗口.t", std::ios::binary);
    out << kSmokeSource;
  }

  void TearDown() override {
    std::error_code error;
    if (!workspace_.empty())
      fs::remove_all(workspace_, error);
  }

  std::string CompilerPath() const {
    const char* from_env = std::getenv(kCompilerEnvVar);
    if (from_env != nullptr && *from_env != '\0')
      return from_env;
    return (fs::path(TIECODEC_BINARY_DIR) / "tiec").string();
  }

  static std::string UniqueDirectoryName() {
    static int counter = 0;
    return "tiecodec_smoke_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
           "_" + std::to_string(counter++);
  }

  ProcessResult RunCompiler(const std::vector<std::string>& extra_args = {}) {
    std::vector<fs::path> sources = CollectSources(stdlib_);
    const std::vector<fs::path> own = CollectSources(workspace_ / "src");
    sources.insert(sources.end(), own.begin(), own.end());

    std::vector<std::string> arguments = {compiler_.string(), "--platform", "android", "--release",
                                          "--friendly-name",  "original",   "-o",      "out"};
    arguments.insert(arguments.end(), extra_args.begin(), extra_args.end());
    for (const fs::path& source : sources)
      arguments.push_back(source.string());
    return SpawnCompiler(std::move(arguments));
  }

  ProcessResult SpawnCompiler(std::vector<std::string> arguments) {
    std::vector<char*> argv;
    argv.reserve(arguments.size() + 1);
    for (std::string& argument : arguments)
      argv.push_back(argument.data());
    argv.push_back(nullptr);

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    const std::string log_path = (workspace_ / "compile.log").string();
    posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, log_path.c_str(),
                                     O_WRONLY | O_CREAT | O_TRUNC, 0644);
    posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO);

    const std::string previous_directory = fs::current_path().string();
    fs::current_path(workspace_);

    pid_t pid = 0;
    const int spawned =
        posix_spawn(&pid, compiler_.c_str(), &actions, nullptr, argv.data(), environ);
    fs::current_path(previous_directory);
    posix_spawn_file_actions_destroy(&actions);

    ProcessResult result;
    if (spawned != 0) {
      result.status = -1;
      result.log = std::string("无法启动编译器: ") + std::strerror(spawned) + "\n";
      return result;
    }

    int wait_status = 0;
    while (waitpid(pid, &wait_status, 0) == -1 && errno == EINTR) {
    }
    result.status = WIFEXITED(wait_status) ? WEXITSTATUS(wait_status) : -1;
    result.log = ReadFile(workspace_ / "compile.log");
    return result;
  }

  fs::path compiler_;
  fs::path stdlib_;
  fs::path workspace_;
};

TEST_F(SmokeTest, CompilesStarterWindow) {
  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << "冒烟编译失败，日志:\n" << result.log.substr(0, 2000);

  const fs::path generated = workspace_ / "out" / "src" / "启动窗口.java";
  ASSERT_TRUE(fs::is_regular_file(generated)) << "冒烟编译未生成 启动窗口.java，日志:\n"
                                              << result.log.substr(0, 2000);

  const std::string java = ReadFile(generated);
  EXPECT_TRUE(Contains(java, "public void 创建完毕()")) << "产物缺少事件方法";
}

TEST_F(SmokeTest, CheckFailsForUnreadableSource) {
  const fs::path missing = workspace_ / "missing.t";
  const ProcessResult result = SpawnCompiler({compiler_.string(), "--check", missing.string()});

  EXPECT_NE(result.status, 0) << "读取失败时 --check 不应返回成功，日志:\n" << result.log;
  EXPECT_TRUE(Contains(result.log, "无法读取源文件")) << "日志缺少读取错误:\n" << result.log;
  EXPECT_TRUE(Contains(result.log, "检查失败")) << "日志缺少失败提示:\n" << result.log;
  EXPECT_FALSE(Contains(result.log, "检查通过")) << "--check 不应误报检查通过:\n" << result.log;
}

TEST_F(SmokeTest, RejectsPathTraversalOutputName) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "@输出名(\"../escape\")\n"
            "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  EXPECT_NE(result.status, 0) << "路径穿越输出名应编译失败，日志:\n" << result.log;
  EXPECT_TRUE(Contains(result.log, "类输出名")) << result.log;
  EXPECT_FALSE(fs::exists(workspace_ / "escape.java")) << "不得写出输出目录之外的文件";
}

TEST_F(SmokeTest, CheckRequiresSourceFiles) {
  const ProcessResult result = SpawnCompiler({compiler_.string(), "--check"});
  EXPECT_NE(result.status, 0) << "空源文件列表不应返回成功，日志:\n" << result.log;
  EXPECT_TRUE(Contains(result.log, "没有待编译的源文件")) << result.log;
}

TEST_F(SmokeTest, RejectsUnknownFriendlyName) {
  const ProcessResult result =
      SpawnCompiler({compiler_.string(), "--friendly-name", "bogus", "--check"});
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "无效的命名模式")) << result.log;
}

TEST_F(SmokeTest, RejectsUnsupportedPlatform) {
  const ProcessResult result =
      SpawnCompiler({compiler_.string(), "--platform", "harmony", "missing.t"});
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "不支持的目标平台")) << result.log;
}

TEST_F(SmokeTest, EmitsLegalGenericSpecializationNames) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 列表: 文本集合\n"
            "  事件 启动窗口:创建完毕()\n"
            "    列表.添加成员(\"x\")\n"
            "    调试输出(列表[0])\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const fs::path src_dir = workspace_ / "out" / "src";
  ASSERT_TRUE(fs::is_directory(src_dir)) << result.log;

  bool found_specialization = false;
  for (const fs::directory_entry& entry : fs::directory_iterator(src_dir)) {
    const std::string name = entry.path().filename().string();
    EXPECT_EQ(name.find('<'), std::string::npos) << name;
    EXPECT_EQ(name.find('>'), std::string::npos) << name;
    if (name == "集合$文本.java")
      found_specialization = true;
  }
  EXPECT_TRUE(found_specialization) << "未生成 集合$文本.java";
  EXPECT_FALSE(fs::exists(src_dir / "集合.java")) << "不应生成别名泛型裸壳";

  const fs::path specialization = src_dir / "集合$文本.java";
  ASSERT_TRUE(fs::is_regular_file(specialization)) << result.log;
  const std::string spec = ReadFile(specialization);
  EXPECT_TRUE(Contains(spec, "__this_obj")) << spec;
  EXPECT_EQ(spec.find("集合.this"), std::string::npos) << spec;

  const fs::path launcher = src_dir / "启动窗口.java";
  ASSERT_TRUE(fs::is_regular_file(launcher)) << result.log;
  const std::string java = ReadFile(launcher);
  EXPECT_TRUE(Contains(java, "集合$文本.添加成员")) << java;
  EXPECT_TRUE(Contains(java, "operator_array_access")) << java;
  EXPECT_EQ(java.find("import 结绳.JVM.集合<"), std::string::npos) << java;
}

TEST_F(SmokeTest, EmitsFixedSizeArrayField) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 数字: 整数[3]\n"
            "  事件 启动窗口:创建完毕()\n"
            "    数字[0] = 1\n"
            "    调试输出(数字[0])\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "new int[3]")) << java;
  EXPECT_TRUE(Contains(java, "数字[0] = 1")) << java;
}

TEST_F(SmokeTest, InitializesAliasedReferenceFields) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "@指代类(\"java.util.ArrayList\")\n"
            "类 列表类\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  变量 文本字段: 文本\n"
            "  变量 对象字段: 对象\n"
            "  变量 列表字段: 列表类\n"
            "  事件 启动窗口:创建完毕()\n"
            "    调试输出(文本字段)\n"
            "    调试输出(对象字段)\n"
            "    调试输出(列表字段)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "文本字段 = \"\"")) << java;
  EXPECT_TRUE(Contains(java, "对象字段 = new Object()")) << java;
  EXPECT_TRUE(Contains(java, "列表字段 = new java.util.ArrayList()")) << java;
}

TEST_F(SmokeTest, ResolvesShadowingFieldProperty) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 相机\n"
            "  属性读 值(): 整数\n"
            "    返回 1\n"
            "  结束 属性\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  变量 相机: 相机 = 创建 相机()\n"
            "  事件 启动窗口:创建完毕()\n"
            "    调试输出(相机.值)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "相机.值()")) << java;
}

TEST_F(SmokeTest, ReportsFullWidthPunctuation) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 值: 整数 = 1，\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "请使用半角符号")) << result.log;
}

TEST_F(SmokeTest, AcceptsUtf8CharLiteral) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 字: 字符 = '中'\n"
            "  事件 启动窗口:创建完毕()\n"
            "    调试输出(字)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const fs::path generated = workspace_ / "out" / "src" / "启动窗口.java";
  ASSERT_TRUE(fs::is_regular_file(generated)) << result.log;
  EXPECT_TRUE(Contains(ReadFile(generated), "'中'")) << ReadFile(generated);
}

TEST_F(SmokeTest, InitializesLocalVariablesLikeFields) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 用户\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 整数变量: 整数\n"
            "    变量 文本变量: 文本\n"
            "    变量 对象变量: 对象\n"
            "    变量 用户变量: 用户\n"
            "    调试输出(整数变量)\n"
            "    调试输出(文本变量)\n"
            "    调试输出(对象变量)\n"
            "    调试输出(用户变量)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "int 整数变量 = 0;")) << java;
  EXPECT_TRUE(Contains(java, "String 文本变量 = \"\";")) << java;
  EXPECT_TRUE(Contains(java, "Object 对象变量 = new Object();")) << java;
  EXPECT_TRUE(Contains(java, "用户 用户变量 = new 用户();")) << java;
}

TEST_F(SmokeTest, NoArgForcedOutputKeepsClass) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "@强制输出\n"
            "类 辅助\n"
            "  方法 未使用()\n"
            "  结束 方法\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  EXPECT_TRUE(fs::exists(workspace_ / "out" / "src" / "辅助.java")) << result.log;
}

TEST_F(SmokeTest, DispatchesExternalTextEquality) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 文本变量: 文本 = \"a\"\n"
            "  事件 启动窗口:创建完毕()\n"
            "    如果 文本变量 == \"a\"\n"
            "      调试输出(\"eq\")\n"
            "    结束 如果\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "文本.operator_equal(文本变量, \"a\")")) << java;
}

TEST_F(SmokeTest, DispatchesExternalTextContains) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 文本变量: 文本 = \"abc\"\n"
            "  事件 启动窗口:创建完毕()\n"
            "    如果 文本变量 ? \"b\"\n"
            "      调试输出(\"contains\")\n"
            "    结束 如果\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "文本.operator_question(文本变量, \"b\")")) << java;
}

TEST_F(SmokeTest, FoldsIntegerOverflowAsInt32) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  常量 大数: 整数 = 2147483647 + 1\n"
            "  事件 启动窗口:创建完毕()\n"
            "    调试输出(大数)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "public final int 大数 = -2147483648;")) << java;
}

TEST_F(SmokeTest, AcceptsHexLongLiteral) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 值: 长整数 = 0xFFL\n"
            "  事件 启动窗口:创建完毕()\n"
            "    调试输出(值)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "public long 值 = 0xFFL;")) << java;
}

TEST_F(SmokeTest, RejectsDecimalLongLiteral) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 值: 小数 = 1.5L\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "此处不应该出现 'L'")) << result.log;
}

TEST_F(SmokeTest, UsesJavaArrayLength) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 数组: 整数[] = {1, 2}\n"
            "  事件 启动窗口:创建完毕()\n"
            "    如果 数组.长度 > 0\n"
            "      调试输出(\"ok\")\n"
            "    结束 如果\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "数组.length")) << java;
}

TEST_F(SmokeTest, CompilesAsyncConditionAndSwitch) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  @异步方法\n"
            "  方法 加载()\n"
            "    如果 等待 取逻辑()\n"
            "      假如 1\n"
            "        是 1\n"
            "          等待 取数据()\n"
            "        结束 假如\n"
            "    结束 如果\n"
            "  结束 方法\n"
            "  方法 取逻辑(): 逻辑型\n"
            "    返回 真\n"
            "  结束 方法\n"
            "  方法 取数据()\n"
            "  结束 方法\n"
            "  事件 启动窗口:创建完毕()\n"
            "    加载()\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "上下文.rn_")) << java;
  EXPECT_TRUE(Contains(java, "switch (1)")) << java;
}

TEST_F(SmokeTest, SupportsAnnotatedPropertyAccessors) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 辅助\n"
            "  变量 值: 文本 = \"x\"\n"
            "  @属性读\n"
            "  方法 标题(): 文本\n"
            "    返回 值\n"
            "  结束 方法\n"
            "  @属性写\n"
            "  方法 标题(新值: 文本)\n"
            "    值 = 新值\n"
            "  结束 方法\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  变量 对象: 辅助 = 创建 辅助\n"
            "  事件 启动窗口:创建完毕()\n"
            "    调试输出(对象.标题)\n"
            "    对象.标题 = \"y\"\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "对象.标题()")) << java;
  EXPECT_TRUE(Contains(java, "对象.标题(\"y\")")) << java;
}

TEST_F(SmokeTest, ReadsOctalIntegerLiteral) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  常量 值: 整数 = 077\n"
            "  事件 启动窗口:创建完毕()\n"
            "    调试输出(值)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "public final int 值 = 63;")) << java;
}

TEST_F(SmokeTest, ForcedTrueKeepsClassMembers) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "@强制输出(真)\n"
            "类 辅助\n"
            "  方法 未使用()\n"
            "  结束 方法\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "辅助.java");
  EXPECT_TRUE(Contains(java, "未使用")) << java;
}

TEST_F(SmokeTest, DisablesFtcNegativeStep) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    循环(i, 10, 0, -1)\n"
            "      调试输出(i)\n"
            "    结束 循环\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler({"--disable-ftc-features"});
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "负步长")) << result.log;
}

TEST_F(SmokeTest, DisablesFtcDuplicateSubscriptions) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 源\n"
            "  定义事件 点击()\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  变量 源对象: 源\n"
            "  事件 源对象:点击()\n"
            "  结束 事件\n"
            "  事件 启动窗口:创建完毕()\n"
            "    订阅事件\n"
            "    订阅事件\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler({"--disable-ftc-features"});
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "重复订阅")) << result.log;
}

TEST_F(SmokeTest, DisablesFtcPrimitiveEventDefault) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  定义事件 字符事件() : 字符\n"
            "  事件 启动窗口:创建完毕()\n"
            "    字符事件()\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler({"--disable-ftc-features"});
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "原始类型返回值")) << result.log;
}

TEST_F(SmokeTest, DisablesFtcGenericFeatures) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 盒子<元素类型 = 对象>\n"
            "  变量 内容: 元素类型\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  变量 盒: 盒子<文本>\n"
            "  事件 启动窗口:创建完毕()\n"
            "    调试输出(盒.内容)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler({"--disable-ftc-features"});
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "禁用 FreeTiecodec 特性")) << result.log;
  EXPECT_TRUE(Contains(result.log, "泛型")) << result.log;
}

TEST_F(SmokeTest, DisablesFtcOperatorFeatures) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 数值\n"
            "  方法 +(其他: 数值): 数值\n"
            "    返回 本对象\n"
            "  结束 方法\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  变量 值: 数值 = 创建 数值\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 结果: 数值 = 值 + 值\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler({"--disable-ftc-features"});
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "运算符重载")) << result.log;
}

TEST_F(SmokeTest, DisablesFtcInlineSemicolon) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  @嵌入式代码\n"
            "  方法 记录(内容: 文本)\n"
            "    code android.util.Log.i(\"T\", #内容)\n"
            "  结束 方法\n"
            "  事件 启动窗口:创建完毕()\n"
            "    记录(\"x\")\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler({"--disable-ftc-features"});
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "内联代码语句")) << result.log;
}

TEST_F(SmokeTest, RequiresTerminatorForNestedElseIf) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    如果 假\n"
            "      调试输出(\"a\")\n"
            "    否则 如果 真\n"
            "      调试输出(\"b\")\n"
            "    结束 如果\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  EXPECT_NE(result.status, 0) << result.log;
}

TEST_F(SmokeTest, SupportsElseIfWithNestedTerminator) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    如果 假\n"
            "      调试输出(\"a\")\n"
            "    否则 如果 真\n"
            "      调试输出(\"b\")\n"
            "    结束 如果\n"
            "    结束 如果\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
}

TEST_F(SmokeTest, EmitsNestedIfBlockForKeywordElse) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 数量: 整数 = 2\n"
            "    如果 数量 > 3\n"
            "      调试输出(\"大\")\n"
            "    否则 如果 数量 > 1\n"
            "      调试输出(\"中\")\n"
            "    结束 如果\n"
            "    结束 如果\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "} else {\n      if (数量 > 1) {")) << java;
}

TEST_F(SmokeTest, EmitsInlineIfForConditionalElse) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 数量: 整数 = 2\n"
            "    如果 数量 > 3\n"
            "      调试输出(\"大\")\n"
            "    否则 数量 > 1\n"
            "      调试输出(\"中\")\n"
            "    结束 如果\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "} else {if (数量 > 1) {")) << java;
}

TEST_F(SmokeTest, SupportsElseBlockWithNestedIf) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    如果 假\n"
            "      调试输出(\"a\")\n"
            "    否则\n"
            "      如果 真\n"
            "        调试输出(\"b\")\n"
            "      结束 如果\n"
            "    结束 如果\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
}

TEST_F(SmokeTest, CompilesDeepConditionalElseChain) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 数量: 整数 = 2\n"
            "    如果 数量 == 0\n"
            "      调试输出(\"首\")\n";
  for (int i = 1; i < 60000; ++i) {
    source << "    否则 数量 == " << i
           << "\n"
              "      调试输出(\"中\")\n";
  }
  source << "    否则\n"
            "      调试输出(\"尾\")\n"
            "    结束 如果\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "} else {if (数量 == 59999) {")) << "深链未被展平发射";
  EXPECT_TRUE(Contains(java, "String.valueOf(\"尾\")")) << java.substr(java.size() - 2000);
}

TEST_F(SmokeTest, ResolvesTransitiveLocalUsage) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 甲: 整数 = 1\n"
            "    变量 乙: 整数 = 甲 + 1\n"
            "    调试输出(乙)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  EXPECT_FALSE(Contains(result.log, "未被使用")) << result.log;
}

TEST_F(SmokeTest, RemovesUnusedLocalChains) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 甲: 整数 = 1\n"
            "    变量 乙: 整数 = 甲 + 1\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  std::size_t warnings = 0;
  std::size_t cursor = result.log.find("未被使用");
  while (cursor != std::string::npos) {
    ++warnings;
    cursor = result.log.find("未被使用", cursor + 1);
  }
  EXPECT_EQ(warnings, 2u) << result.log;
}

TEST_F(SmokeTest, HandlesMapKeyValueForeach) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 映射: 哈希表<文本, 整数> = 创建 哈希表<文本, 整数>\n"
            "  事件 启动窗口:创建完毕()\n"
            "    循环(映射 -> 键, 值)\n"
            "      调试输出(键)\n"
            "      调试输出(值)\n"
            "    结束 循环\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "java.util.Map.Entry<String, Integer>")) << java;
  EXPECT_TRUE(Contains(java, ".entrySet()")) << java;
  EXPECT_TRUE(Contains(java, ".getKey()")) << java;
  EXPECT_TRUE(Contains(java, ".getValue()")) << java;
}

TEST_F(SmokeTest, SupportsNegativeCountStep) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    循环(i, 10, 0, -1)\n"
            "      调试输出(i)\n"
            "    结束 循环\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, " > 0;")) << java;
}

TEST_F(SmokeTest, SwapsObjectIndexers) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 集合甲: 集合<文本> = 创建 集合<文本>\n"
            "  变量 集合乙: 集合<文本> = 创建 集合<文本>\n"
            "  事件 启动窗口:创建完毕()\n"
            "    集合甲[0] <=> 集合乙[0]\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "集合甲")) << java;
  EXPECT_TRUE(Contains(java, "集合乙")) << java;
  EXPECT_TRUE(Contains(java, "operator_array_assign")) << java;
}

TEST_F(SmokeTest, DeduplicatesSubscriptions) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 源\n"
            "  定义事件 点击()\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  变量 源对象: 源\n"
            "  事件 源对象:点击()\n"
            "    调试输出(\"点击\")\n"
            "  结束 事件\n"
            "  事件 启动窗口:创建完毕()\n"
            "    订阅事件\n"
            "    订阅事件\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  const std::size_t first = java.find("源对象.set点击Listener");
  ASSERT_NE(first, std::string::npos) << java;
  EXPECT_EQ(java.find("源对象.set点击Listener", first + 1), std::string::npos) << java;
}

TEST_F(SmokeTest, EmitsPrimitiveEventDefaults) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  定义事件 字符事件() : 字符\n"
            "  事件 启动窗口:创建完毕()\n"
            "    字符事件()\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "return '\\0';")) << java;
}

TEST_F(SmokeTest, RejectsInvalidMemberOutputName) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  @输出名(\"1bad\")\n"
            "  变量 字段: 整数 = 1\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "成员输出名")) << result.log;
}

TEST(OutputSinkTest, RejectsPathTraversal) {
  const fs::path root = fs::temp_directory_path() / ("tiecodec_sink_" + std::to_string(::getpid()));
  fs::remove_all(root);
  fs::create_directories(root);
  FileSystemOutputSink sink(root.string());

  EXPECT_FALSE(sink.Write("../escape.txt", "x"));
  EXPECT_FALSE(sink.CopyFile(__FILE__, "../escape.txt"));
  EXPECT_FALSE(fs::exists(root.parent_path() / "escape.txt"));
  EXPECT_TRUE(sink.Write("nested/file.txt", "ok"));
  EXPECT_TRUE(fs::exists(root / "nested" / "file.txt"));
  fs::remove_all(root);
}

TEST_F(SmokeTest, ReportsUnclosedCodeBlock) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    @code\n"
            "    int x = 1;\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "未闭合的 code 块")) << result.log;
}

TEST_F(SmokeTest, EmitsHiddenClassWithoutPublicModifier) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "@隐藏\n"
            "类 辅助\n"
            "  变量 值: 文本 = \"x\"\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  变量 对象: 辅助 = 创建 辅助\n"
            "  事件 启动窗口:创建完毕()\n"
            "    调试输出(对象.值)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "辅助.java");
  EXPECT_TRUE(Contains(java, "class 辅助 {")) << java;
  EXPECT_FALSE(Contains(java, "public class 辅助")) << java;
}

TEST_F(SmokeTest, ReportsDeprecatedMemberUsage) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 辅助\n"
            "  @废弃使用(\"旧接口\")\n"
            "  方法 旧方法(): 文本\n"
            "    返回 \"x\"\n"
            "  结束 方法\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  变量 对象: 辅助 = 创建 辅助\n"
            "  事件 启动窗口:创建完毕()\n"
            "    调试输出(对象.旧方法())\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "已废弃")) << result.log;
}

TEST_F(SmokeTest, AppliesGenericParameterDefaults) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 容器<T = 文本>\n"
            "  变量 值: T\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  变量 项: 容器 = 创建 容器\n"
            "  事件 启动窗口:创建完毕()\n"
            "    调试输出(项.值)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const fs::path generated = workspace_ / "out" / "src" / "容器$文本.java";
  ASSERT_TRUE(fs::is_regular_file(generated)) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "容器$文本")) << java;
}

TEST_F(SmokeTest, UsesAssignmentOperatorForMultiExpression) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 坐标\n"
            "  变量 横坐标: 整数\n"
            "  变量 纵坐标: 整数\n"
            "  @静态\n"
            "  @运算符重载\n"
            "  方法 =(x: 整数, y: 整数): 坐标\n"
            "    变量 结果: 坐标 = 创建 坐标\n"
            "    结果.横坐标 = x\n"
            "    结果.纵坐标 = y\n"
            "    返回 结果\n"
            "  结束 方法\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 点: 坐标 = (1, 2)\n"
            "    调试输出(点.横坐标)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "坐标.operator_assign(1, 2)")) << java;
}

TEST_F(SmokeTest, CompilesAsyncCountLoopWithStep) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  @异步方法\n"
            "  方法 加载()\n"
            "    循环(i, 10, 0, -2)\n"
            "      等待 取数据()\n"
            "      调试输出(i)\n"
            "    结束 循环\n"
            "  结束 方法\n"
            "  方法 取数据()\n"
            "  结束 方法\n"
            "  事件 启动窗口:创建完毕()\n"
            "    加载()\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, " >= 0 ? ")) << java;
  EXPECT_TRUE(Contains(java, " > 0)")) << java;
}

TEST_F(SmokeTest, CompilesAsyncForeachWithAwait) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  @异步方法\n"
            "  方法 加载()\n"
            "    变量 列表: 集合<文本> = 创建 集合<文本>\n"
            "    循环(列表 -> 条目)\n"
            "      等待 取数据()\n"
            "      调试输出(条目)\n"
            "    结束 循环\n"
            "  结束 方法\n"
            "  方法 取数据()\n"
            "  结束 方法\n"
            "  事件 启动窗口:创建完毕()\n"
            "    加载()\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler({"--line-map", "out/map.tsm"});
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string context =
      ReadFile(workspace_ / "out" / "src" / "启动窗口_加载_异步上下文.java");
  EXPECT_TRUE(Contains(context, "java.util.Iterator<String>")) << context;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, ".hasNext()")) << java;
  EXPECT_TRUE(Contains(java, ".next()")) << java;
  ASSERT_TRUE(fs::is_regular_file(workspace_ / "out" / "map.tsm"));

  const ProcessResult retrace =
      SpawnCompiler({compiler_.string(), "--retrace", (workspace_ / "out" / "map.tsm").string(),
                     "启动窗口_加载_异步上下文.java", "5"});
  EXPECT_TRUE(Contains(retrace.log, "启动窗口.t:")) << retrace.log;
}

TEST_F(SmokeTest, RejectsBreakOutsideLoop) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    退出循环\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "只能在循环内部使用")) << result.log;
}

TEST_F(SmokeTest, RejectsPrimitiveInstanceof) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 值: 整数 = 1\n"
            "    如果 值 属于 整数\n"
            "      调试输出(\"整数\")\n"
            "    结束 如果\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "原始类型")) << result.log;
}

TEST_F(SmokeTest, IgnoresEndMarkerInsideCodeComment) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    @code\n"
            "    /*\n"
            "@end\n"
            "    */\n"
            "    int manual = 1;\n"
            "    @end\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "int manual = 1;")) << java;
}

TEST_F(SmokeTest, TreatsDottedCodeIdentifierAsField) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 辅助\n"
            "  变量 code: 文本 = \"x\"\n"
            "结束 类\n"
            "类 启动窗口 : 窗口\n"
            "  变量 对象: 辅助 = 创建 辅助\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 值: 文本 = 对象.code\n"
            "    调试输出(值)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "对象.code")) << java;
}

TEST_F(SmokeTest, EmitsUntypedArrayLiteralAsObjectArray) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 值: 对象 = {1, 2, 3}\n"
            "    调试输出(值)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler();
  ASSERT_EQ(result.status, 0) << result.log;
  const std::string java = ReadFile(workspace_ / "out" / "src" / "启动窗口.java");
  EXPECT_TRUE(Contains(java, "new Object[]{1, 2, 3}")) << java;
}

TEST_F(SmokeTest, ReportsUnclosedBlockCommentAndLongString) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  /* 未闭合\n"
            "结束 类\n";
  source.close();

  const ProcessResult comment = RunCompiler();
  EXPECT_NE(comment.status, 0) << comment.log;
  EXPECT_TRUE(Contains(comment.log, "未闭合的块注释")) << comment.log;

  source.open(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  变量 文本值: 文本 = [[未闭合\n"
            "结束 类\n";
  source.close();

  const ProcessResult text = RunCompiler();
  EXPECT_NE(text.status, 0) << text.log;
  EXPECT_TRUE(Contains(text.log, "未闭合的长字符串")) << text.log;
}

TEST_F(SmokeTest, CheckRejectsUnresolvedGenericArgument) {
  std::ofstream source(workspace_ / "src" / "启动窗口.t", std::ios::binary | std::ios::trunc);
  source << "类 启动窗口 : 窗口\n"
            "  事件 启动窗口:创建完毕()\n"
            "    变量 列表: 集合<不存在类型> = 创建 集合<不存在类型>\n"
            "    调试输出(列表)\n"
            "  结束 事件\n"
            "结束 类\n";
  source.close();

  const ProcessResult result = RunCompiler({"--check"});
  EXPECT_NE(result.status, 0) << result.log;
  EXPECT_TRUE(Contains(result.log, "找不到类\"不存在类型\"")) << result.log;
}

}
}
}
