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
    return (fs::path(TIECODEC_BINARY_DIR) / "tiecodec").string();
  }

  static std::string UniqueDirectoryName() {
    static int counter = 0;
    return "tiecodec_smoke_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
           "_" + std::to_string(counter++);
  }

  ProcessResult RunCompiler() {
    std::vector<fs::path> sources = CollectSources(stdlib_);
    const std::vector<fs::path> own = CollectSources(workspace_ / "src");
    sources.insert(sources.end(), own.begin(), own.end());

    std::vector<std::string> arguments = {compiler_.string(), "--platform", "android", "--release",
                                          "--friendly-name",  "original",   "-o",      "out"};
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
  EXPECT_TRUE(Contains(result.log, "解析失败")) << "日志缺少失败提示:\n" << result.log;
  EXPECT_FALSE(Contains(result.log, "解析通过")) << "--check 不应误报解析通过:\n" << result.log;
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

}
}
}
