#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "text_util.h"

extern char** environ;

namespace tiecodec {
namespace test {
namespace {

namespace fs = std::filesystem;

constexpr const char* kCompilerEnvVar = "TIECODEC_TEST_COMPILER";

constexpr const char* kCxxEntrySource = R"(类 动物
结束 类
类 狗 : 动物
结束 类
类 启动类
    变量 计数: 整数 = 2
    方法 启动方法()
        @导入头文件("<iostream>")
        调试输出(计数)
        如果 计数 > 1 则
            调试输出("大")
        否则
            调试输出("小")
        结束 如果
        变量 甲: 对象 = 创建 狗
        如果 甲 属于 动物
            调试输出("是动物")
        结束 如果
        变量 文: 文本 = "乙"
        假如 文
        是 "甲"
            调试输出("一")
        是 "乙"
            调试输出("二")
        结束 假如
        退出程序()
    结束 方法
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

int Spawn(const std::vector<std::string>& arguments, const fs::path& log_path) {
  std::vector<char*> argv;
  argv.reserve(arguments.size() + 1);
  for (const std::string& argument : arguments)
    argv.push_back(const_cast<char*>(argument.c_str()));
  argv.push_back(nullptr);

  posix_spawn_file_actions_t actions;
  posix_spawn_file_actions_init(&actions);
  posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, log_path.c_str(),
                                   O_WRONLY | O_CREAT | O_TRUNC, 0644);
  posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO);

  pid_t pid = 0;
  const int spawned = posix_spawnp(&pid, argv[0], &actions, nullptr, argv.data(), environ);
  posix_spawn_file_actions_destroy(&actions);
  if (spawned != 0)
    return -1;

  int wait_status = 0;
  while (waitpid(pid, &wait_status, 0) == -1 && errno == EINTR) {
  }
  return WIFEXITED(wait_status) ? WEXITSTATUS(wait_status) : -1;
}

bool HasCxxCompiler() {
  const fs::path log = fs::temp_directory_path() / "tiecodec_cxx_tool.log";
  const int status = Spawn({"c++", "--version"}, log);
  std::error_code error;
  fs::remove(log, error);
  return status == 0;
}

class CxxSmokeTest : public ::testing::Test {
protected:
  void SetUp() override {
    const char* from_env = std::getenv(kCompilerEnvVar);
    compiler_ = from_env != nullptr && *from_env != '\0' ? fs::path(from_env)
                                                         : fs::path(TIECODEC_BINARY_DIR) / "tiec";
    ASSERT_TRUE(fs::is_regular_file(compiler_)) << "缺少编译器: " << compiler_;

    stdlib_ = fs::path(TIECODEC_SOURCE_DIR) / "stdlibs" / "CXX基本库" / "源代码";
    ASSERT_TRUE(fs::is_directory(stdlib_)) << "缺少 CXX 基本库: " << stdlib_;

    workspace_ = fs::temp_directory_path() / UniqueDirectoryName();
    fs::create_directories(workspace_ / "src");
    std::ofstream out(workspace_ / "src" / "启动类.t", std::ios::binary);
    out << kCxxEntrySource;
  }

  void TearDown() override {
    std::error_code error;
    if (!workspace_.empty())
      fs::remove_all(workspace_, error);
  }

  static std::string UniqueDirectoryName() {
    static int counter = 0;
    return "tiecodec_cxx_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
           "_" + std::to_string(counter++);
  }

  int RunCompiler(std::string* log, const std::vector<std::string>& extra_arguments = {}) {
    std::vector<std::string> arguments = {compiler_.string(),
                                          "--platform",
                                          "cxx",
                                          "--language",
                                          "cxx",
                                          "--release",
                                          "--friendly-name",
                                          "original",
                                          "-o",
                                          (workspace_ / "out").string()};
    for (const std::string& extra : extra_arguments)
      arguments.push_back(extra);
    for (const fs::path& source : CollectSources(stdlib_))
      arguments.push_back(source.string());
    for (const fs::path& source : CollectSources(workspace_ / "src"))
      arguments.push_back(source.string());

    const fs::path log_path = workspace_ / "compile.log";
    const int status = Spawn(arguments, log_path);
    if (log != nullptr)
      *log = ReadFile(log_path);
    return status;
  }

  fs::path compiler_;
  fs::path stdlib_;
  fs::path workspace_;
};

TEST_F(CxxSmokeTest, EmitsReferenceLayoutProject) {
  std::string log;
  ASSERT_EQ(RunCompiler(&log), 0) << "C++ 编译失败:\n" << log.substr(0, 2000);

  const fs::path out = workspace_ / "out";
  const fs::path src = out / "src";
  ASSERT_TRUE(fs::is_regular_file(src / "启动类.h")) << "缺少头文件";
  ASSERT_TRUE(fs::is_regular_file(src / "启动类.cpp")) << "缺少实现文件";
  ASSERT_TRUE(fs::is_regular_file(src / "main.cpp")) << "缺少启动器";
  ASSERT_TRUE(fs::is_regular_file(src / "CMakeLists.txt"));
  ASSERT_TRUE(fs::is_regular_file(src / "commands-g++.sh"));
  ASSERT_TRUE(fs::is_regular_file(src / "msvc.bat"));
  ASSERT_TRUE(fs::is_regular_file(src / "tie_cxx_foundation.h"));
  ASSERT_TRUE(fs::is_regular_file(src / "tie_cxx_foundation.cpp"));
  ASSERT_TRUE(fs::is_regular_file(src / "tie_cxx_thread.h"));
  ASSERT_TRUE(fs::is_regular_file(out / "metadata.json"));

  const std::string header_text = ReadFile(src / "启动类.h");
  EXPECT_TRUE(Contains(header_text, "#ifndef TIECODEC_GEN_"));
  EXPECT_TRUE(Contains(header_text, "#include \"tie_cxx_foundation.h\""));
  EXPECT_TRUE(Contains(header_text, "using namespace tie;"));
  EXPECT_TRUE(Contains(header_text, "namespace 结绳::中文"));
  EXPECT_TRUE(Contains(header_text, "class 启动类 : public CTieObject"));
  EXPECT_TRUE(Contains(header_text, "T_INT 计数;"));
  EXPECT_TRUE(Contains(header_text, "T_STRING ToString() override;"));

  const std::string source_text = ReadFile(src / "启动类.cpp");
  EXPECT_TRUE(Contains(source_text, "#include \"启动类.h\""));
  EXPECT_TRUE(Contains(source_text, "启动类::启动类(): 计数(2)"));
  EXPECT_TRUE(Contains(source_text, "if (计数 > 1) {"));
  EXPECT_TRUE(Contains(source_text, "} else {"));
  EXPECT_TRUE(Contains(source_text, "std::dynamic_pointer_cast<动物>(甲) != nullptr"));
  EXPECT_TRUE(Contains(source_text, "} else if (tie_情景1 == _TSTR(\"乙\")) {"));
  const std::string base_header = ReadFile(src / "狗.h");
  EXPECT_TRUE(Contains(base_header, "class 狗 : public 动物"));

  const std::string launcher_text = ReadFile(src / "main.cpp");
  EXPECT_TRUE(Contains(launcher_text, "#include \"tie_cxx_thread.h\""));
  EXPECT_TRUE(Contains(launcher_text, "GetMainLooper().PreScheduleMessage(0"));
  EXPECT_TRUE(Contains(launcher_text, "结绳::中文::启动类 entry"));
  EXPECT_TRUE(Contains(launcher_text, "entry.启动方法();"));

  const std::string metadata = ReadFile(out / "metadata.json");
  EXPECT_TRUE(Contains(metadata, "\"language\": \"cxx\""));
  EXPECT_TRUE(Contains(metadata, "结绳::中文::启动类"));
}

TEST_F(CxxSmokeTest, CompatModeReportsFtcErrors) {
  std::string log;
  ASSERT_NE(RunCompiler(&log, {"--disable-ftc-features"}), 0)
      << "兼容模式应报错而非产出不可编译产物:\n"
      << log.substr(0, 2000);
  EXPECT_TRUE(Contains(log, "多段包名的参考形态")) << log.substr(0, 2000);
  EXPECT_TRUE(Contains(log, "根类缺少 CTieObject 基类")) << log.substr(0, 2000);
  EXPECT_TRUE(Contains(log, "属于判断的参考形态")) << log.substr(0, 2000);
  EXPECT_TRUE(Contains(log, "文本主体的情景判断参考形态")) << log.substr(0, 2000);
}

constexpr const char* kCxxEventAsyncSource = R"(类 发射器
    定义事件 触发(来源: 对象)
结束 类
类 启动类 : 对象
    变量 发: 发射器 = 创建 发射器
    变量 累计: 整数 = 0
    事件 发:触发(来源: 对象)
        调试输出("收到事件")
    结束 事件
    @异步方法
    方法 跑一遍()
        等待 本对象.取数()
        累计 = 42
        调试输出(累计)
        退出程序()
    结束 方法
    方法 取数(): 整数
        返回 7
    结束 方法
    方法 启动方法()
        @导入头文件("<iostream>")
        订阅事件()
        发.触发(空)
        本对象.跑一遍()
    结束 方法
结束 类
)";

TEST_F(CxxSmokeTest, EventsAsyncAndLineMap) {
  std::ofstream src(workspace_ / "src" / "启动类.t", std::ios::binary);
  src << kCxxEventAsyncSource;
  src.close();

  std::vector<std::string> extra = {"--line-map", (workspace_ / "map.tsm").string()};
  std::string log;
  ASSERT_EQ(RunCompiler(&log, extra), 0) << "事件/异步编译失败:\n" << log.substr(0, 2000);

  const fs::path out = workspace_ / "out";
  const fs::path src_dir = out / "src";
  const std::string emitter_header = ReadFile(src_dir / "发射器.h");
  EXPECT_TRUE(Contains(emitter_header, "bind_触发")) << emitter_header;
  EXPECT_TRUE(Contains(emitter_header, "std::function"));
  const std::string launcher_header = ReadFile(src_dir / "启动类.h");
  EXPECT_TRUE(Contains(launcher_header, "发_触发"));
  EXPECT_TRUE(Contains(launcher_header, "跑一遍_异步"));
  EXPECT_TRUE(Contains(launcher_header, "tie_cxx_thread.h"));
  const std::string machine = ReadFile(src_dir / "启动类.cpp");
  EXPECT_TRUE(Contains(machine, "bind_触发([this]")) << machine;
  EXPECT_TRUE(Contains(machine, "GetGlobalExecutor()->Enqueue([=]()"));
  EXPECT_TRUE(Contains(machine, "GetMainLooper().PostMessage(0, [=]()"));
  EXPECT_TRUE(fs::is_regular_file(src_dir / "启动类_跑一遍_异步上下文.h"));
  EXPECT_TRUE(fs::is_regular_file(src_dir / "启动类_跑一遍_异步上下文.cpp"));
  EXPECT_TRUE(fs::is_regular_file(workspace_ / "map.tsm"));
  EXPECT_TRUE(Contains(ReadFile(workspace_ / "map.tsm"), "TSM"));

  if (!HasCxxCompiler())
    return;
  const fs::path build = workspace_ / "build";
  fs::create_directories(build);
  const fs::path cmake_log = workspace_ / "cmake.log";
  ASSERT_EQ(Spawn({"cmake", "-S", (out / "src").string(), "-B", build.string()}, cmake_log), 0)
      << ReadFile(cmake_log);
  const fs::path make_log = workspace_ / "make.log";
  ASSERT_EQ(Spawn({"cmake", "--build", build.string()}, make_log), 0) << ReadFile(make_log);
  const fs::path run_log = workspace_ / "run.log";
  ASSERT_EQ(Spawn({(build / "main").string()}, run_log), 0) << ReadFile(run_log);
  const std::string output = ReadFile(run_log);
  EXPECT_TRUE(Contains(output, "收到事件")) << output;
  EXPECT_TRUE(Contains(output, "42")) << output;
}

TEST_F(CxxSmokeTest, NamingModesEmitPinyinAndRandom) {
  const auto compile_source = [&](const char* source, const std::vector<std::string>& extra) {
    std::error_code ignored;
    fs::remove(workspace_ / "src" / "启动类.t", ignored);
    std::ofstream out(workspace_ / "src" / "启动类.t", std::ios::binary);
    out << source;
    out.close();
    fs::remove_all(workspace_ / "out", ignored);
    std::string log;
    const int status = RunCompiler(&log, extra);
    EXPECT_EQ(status, 0) << log.substr(0, 2000);
    return log;
  };
  std::string log = compile_source(kCxxEntrySource, {"--friendly-name", "pinyin"});
  const fs::path pinyin_src = workspace_ / "out" / "src";
  ASSERT_TRUE(fs::is_regular_file(pinyin_src / "QiDongLei.h")) << log;
  const std::string header = ReadFile(pinyin_src / "QiDongLei.h");
  EXPECT_TRUE(Contains(header, "namespace js_zw")) << header;
  EXPECT_TRUE(Contains(header, "T_INT jiShu;"));
  EXPECT_TRUE(Contains(ReadFile(pinyin_src / "QiDongLei.cpp"), "qiDongFangFa"));

  log = compile_source(kCxxEntrySource, {"--friendly-name", "random"});
  const fs::path random_src = workspace_ / "out" / "src";
  const std::string launcher = ReadFile(random_src / "main.cpp");
  EXPECT_TRUE(Contains(launcher, "rn_")) << launcher << "\n" << log;
  EXPECT_TRUE(Contains(launcher, "::rn_"));
}

TEST_F(CxxSmokeTest, CompatModeReproducesLegalShapes) {
  constexpr const char* kCompatSource = R"(包名 甲
类 启动类 : 对象
    变量 计数: 整数 = 2
    方法 启动方法()
        @导入头文件("<iostream>")
        code std::cout << "甲" << std::endl;
        如果 计数 > 1
            code std::cout << "大" << std::endl;
        否则
            code std::cout << "小" << std::endl;
        结束 如果
    结束 方法
结束 类
)";
  std::ofstream src(workspace_ / "src" / "启动类.t", std::ios::binary);
  src << kCompatSource;
  src.close();
  std::string log;
  const int status = RunCompiler(&log, {"--disable-ftc-features"});
  EXPECT_NE(status, 0) << "启动器参考形态仍应报错";
  EXPECT_NE(log.find("启动器的参考形态缺少线程头包含"), std::string::npos) << log.substr(0, 2000);
  EXPECT_EQ(log.find("compat"), std::string::npos)
      << "不应触发其它兼容错误: " << log.substr(0, 2000);

  const fs::path header = workspace_ / "out" / "src" / "启动类.h";
  ASSERT_TRUE(fs::is_regular_file(header)) << log.substr(0, 2000);
  const std::string header_text = ReadFile(header);
  EXPECT_TRUE(Contains(header_text, "#ifndef 甲_启动类_H")) << header_text;
  EXPECT_TRUE(Contains(header_text, "namespace 甲 {"));
  EXPECT_TRUE(Contains(header_text,
                       "class 启动类 : public CTieObject, std::enable_shared_from_this<启动类>"))
      << header_text;
  const std::string impl_text = ReadFile(workspace_ / "out" / "src" / "启动类.cpp");
  EXPECT_TRUE(Contains(impl_text, "} else \n")) << impl_text;
}

TEST_F(CxxSmokeTest, CompilesAndRunsCxxEntry) {
  std::string log;
  ASSERT_EQ(RunCompiler(&log), 0) << "C++ 编译失败:\n" << log.substr(0, 2000);
  if (!HasCxxCompiler())
    return;

  const fs::path out = workspace_ / "out";
  const fs::path build = workspace_ / "build";
  fs::create_directories(build);

  const fs::path cmake_log = workspace_ / "cmake.log";
  ASSERT_EQ(Spawn({"cmake", "-S", (out / "src").string(), "-B", build.string()}, cmake_log), 0)
      << ReadFile(cmake_log);

  const fs::path make_log = workspace_ / "make.log";
  ASSERT_EQ(Spawn({"cmake", "--build", build.string()}, make_log), 0) << ReadFile(make_log);

  const fs::path binary = build / "main";
  ASSERT_TRUE(fs::is_regular_file(binary)) << "缺少可执行文件";

  const fs::path run_log = workspace_ / "run.log";
  ASSERT_EQ(Spawn({binary.string()}, run_log), 0) << ReadFile(run_log);
  const std::string output = ReadFile(run_log);
  EXPECT_TRUE(Contains(output, "2"));
  EXPECT_TRUE(Contains(output, "大"));
  EXPECT_TRUE(Contains(output, "是动物"));
  EXPECT_TRUE(Contains(output, "二"));
}

}

}

}
