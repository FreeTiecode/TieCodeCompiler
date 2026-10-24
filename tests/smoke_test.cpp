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
    result.log = ReadFile(workspace_ / "compile.log");
    if (spawned != 0) {
      result.status = -1;
      result.log = std::string("无法启动编译器: ") + std::strerror(spawned) + "\n" + result.log;
      return result;
    }

    int wait_status = 0;
    while (waitpid(pid, &wait_status, 0) == -1 && errno == EINTR) {
    }
    result.status = WIFEXITED(wait_status) ? WEXITSTATUS(wait_status) : -1;
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

}
}
}
