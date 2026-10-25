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

constexpr const char* kJvmEntrySource = R"(类 启动类
  方法 启动方法()
    控制台.输出("你好，结绳！")
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

std::vector<fs::path> CollectJavaFiles(const fs::path& directory) {
  std::vector<fs::path> files;
  for (const fs::directory_entry& entry : fs::recursive_directory_iterator(directory)) {
    if (entry.is_regular_file() && EndsWith(entry.path().filename().string(), ".java"))
      files.push_back(entry.path());
  }
  std::sort(files.begin(), files.end());
  return files;
}

struct ProcessResult {
  int status = -1;
  std::string log;
};

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

bool HasJavaTool(const std::string& tool) {
  const fs::path log = fs::temp_directory_path() / ("tiecodec_jvm_tool_" + tool + ".log");
  const int status = Spawn({tool, "-version"}, log);
  std::error_code error;
  fs::remove(log, error);
  return status == 0;
}

class JvmSmokeTest : public ::testing::Test {
protected:
  void SetUp() override {
    const char* from_env = std::getenv(kCompilerEnvVar);
    compiler_ = from_env != nullptr && *from_env != '\0'
                    ? fs::path(from_env)
                    : fs::path(TIECODEC_BINARY_DIR) / "tiecodec";
    ASSERT_TRUE(fs::is_regular_file(compiler_)) << "缺少编译器: " << compiler_;

    stdlib_ = fs::path(TIECODEC_SOURCE_DIR) / "stdlibs" / "安卓基本库" / "源代码" / "Java" /
              "JVM_基本类型.t";
    overlay_ = fs::path(TIECODEC_SOURCE_DIR) / "stdlibs" / "JVM基本库" / "源代码";
    ASSERT_TRUE(fs::is_regular_file(stdlib_)) << "缺少 JVM 基本类型: " << stdlib_;
    ASSERT_TRUE(fs::is_directory(overlay_)) << "缺少 JVM 附加库: " << overlay_;

    workspace_ = fs::temp_directory_path() / UniqueDirectoryName();
    fs::create_directories(workspace_ / "src");
    std::ofstream out(workspace_ / "src" / "启动类.t", std::ios::binary);
    out << kJvmEntrySource;
  }

  void TearDown() override {
    std::error_code error;
    if (!workspace_.empty())
      fs::remove_all(workspace_, error);
  }

  static std::string UniqueDirectoryName() {
    static int counter = 0;
    return "tiecodec_jvm_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
           "_" + std::to_string(counter++);
  }

  ProcessResult RunCompiler() {
    std::vector<std::string> arguments = {
        compiler_.string(), "--platform", "jvm", "--release",
        "--friendly-name",  "original",   "-o",  (workspace_ / "out").string(),
        stdlib_.string()};
    for (const fs::path& source : CollectSources(overlay_))
      arguments.push_back(source.string());
    for (const fs::path& source : CollectSources(workspace_ / "src"))
      arguments.push_back(source.string());

    const fs::path log_path = workspace_ / "compile.log";
    ProcessResult result;
    result.status = Spawn(arguments, log_path);
    result.log = ReadFile(log_path);
    return result;
  }

  fs::path compiler_;
  fs::path stdlib_;
  fs::path overlay_;
  fs::path workspace_;
};

TEST_F(JvmSmokeTest, CompilesAndRunsJvmEntry) {
  const ProcessResult compile = RunCompiler();
  ASSERT_EQ(compile.status, 0) << "JVM 编译失败:\n" << compile.log.substr(0, 2000);

  const fs::path source_dir = workspace_ / "out" / "src";
  ASSERT_TRUE(fs::is_regular_file(source_dir / "启动类.java"));
  ASSERT_TRUE(fs::is_regular_file(source_dir / "Main.java"));
  ASSERT_TRUE(fs::is_regular_file(workspace_ / "out" / "metadata.json"));

  const std::string entry = ReadFile(source_dir / "启动类.java");
  EXPECT_TRUE(Contains(entry, "你好，结绳！"));
  const std::string launcher = ReadFile(source_dir / "Main.java");
  EXPECT_TRUE(Contains(launcher, "启动方法"));

  for (const fs::path& file : CollectJavaFiles(source_dir)) {
    const std::string content = ReadFile(file);
    EXPECT_FALSE(Contains(content, "android.")) << file << " 包含 Android 依赖";
  }

  if (!HasJavaTool("javac") || !HasJavaTool("java"))
    return;

  const fs::path classes = workspace_ / "classes";
  fs::create_directories(classes);
  std::vector<std::string> javac_arguments = {"javac", "-encoding", "UTF-8", "-d",
                                              classes.string()};
  for (const fs::path& file : CollectJavaFiles(source_dir))
    javac_arguments.push_back(file.string());
  const fs::path javac_log = workspace_ / "javac.log";
  ASSERT_EQ(Spawn(javac_arguments, javac_log), 0) << ReadFile(javac_log);

  const fs::path java_log = workspace_ / "java.log";
  const int run_status = Spawn({"java", "-cp", classes.string(), "Main"}, java_log);
  ASSERT_EQ(run_status, 0) << ReadFile(java_log);
  EXPECT_TRUE(Contains(ReadFile(java_log), "你好，结绳！"));
}

}

}

}
