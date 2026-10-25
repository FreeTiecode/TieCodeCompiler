#include "command_line.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "backend_registry.h"
#include "constants.h"
#include "text_util.h"

namespace tiecodec {
namespace {

const std::vector<std::string> kSourceVersions = {"40", "46", "47"};
constexpr int kMinOptimizeLevel = 0;
constexpr int kMaxOptimizeLevel = 3;
constexpr const char* kGradleModeName = "gradle";

using OptionHandler = void (*)(CliCommand&, const std::string&);

struct OptionSpec {
  const char* name;
  const char* alias;
  bool takes_value;
  OptionHandler apply;
};

const OptionSpec kOptions[] = {
    {"-h", "--help", false, [](CliCommand& command, const std::string&) { command.help = true; }},
    {"-o", "--output", true,
     [](CliCommand& command, const std::string& value) { command.output_dir = value; }},
    {"-p", "--package", true,
     [](CliCommand& command, const std::string& value) { command.package_name = value; }},
    {"--dir", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.source_dirs.push_back(value); }},
    {"--platform", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.platform = value; }},
    {"--release", nullptr, false,
     [](CliCommand& command, const std::string&) { command.release = true; }},
    {"--debug", nullptr, false,
     [](CliCommand& command, const std::string&) { command.release = false; }},
    {"--friendly-name", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.friendly_name = value; }},
    {"--app-name", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.app_name = value; }},
    {"--version-name", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.version_name = value; }},
    {"--dump-ast", nullptr, false,
     [](CliCommand& command, const std::string&) { command.dump_ast = true; }},
    {"--check", nullptr, false,
     [](CliCommand& command, const std::string&) { command.check_only = true; }},
    {"--keep-defaults", nullptr, false, [](CliCommand&, const std::string&) {}},
    {"-d", "--define", true,
     [](CliCommand& command, const std::string& value) {
       const std::size_t equals = value.find('=');
       if (equals == std::string::npos) {
         command.defines.emplace_back(value, kLiteralTrue);
         return;
       }
       command.defines.emplace_back(value.substr(0, equals), value.substr(equals + 1));
     }},
    {"--source", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.source_version = value; }},
    {"--hard-mode", nullptr, false,
     [](CliCommand& command, const std::string&) { command.hard_mode = true; }},
    {"--enable-toplevel-stmt", nullptr, false,
     [](CliCommand& command, const std::string&) { command.enable_toplevel_stmt = true; }},
    {"--profile", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.profile = value; }},
    {"--optimize", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.optimize = value; }},
    {"--disable-lint", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.disable_lint.push_back(value); }},
    {"--log-level", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.log_level = value; }},
    {"--emit-names", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.emit_names = value; }},
    {"--stable-names", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.stable_names = value; }},
    {"--search-lib-prefix", nullptr, true,
     [](CliCommand& command, const std::string& value) {
       command.search_lib_prefixes.push_back(value);
     }},
    {"--search-source-prefix", nullptr, true,
     [](CliCommand& command, const std::string& value) {
       command.search_source_prefixes.push_back(value);
     }},
    {"--search-res-prefix", nullptr, true,
     [](CliCommand& command, const std::string& value) {
       command.search_res_prefixes.push_back(value);
     }},
    {"--search-assets-prefix", nullptr, true,
     [](CliCommand& command, const std::string& value) {
       command.search_assets_prefixes.push_back(value);
     }},
    {"--line-map", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.line_map = value; }},
    {"--retrace-batch", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.retrace_batch = value; }},
    {"--android.app.config", nullptr, true,
     [](CliCommand& command, const std::string& value) { command.android_app_config = value; }},
    {"--android.legacy.lib.path", nullptr, true,
     [](CliCommand& command, const std::string& value) {
       command.android_legacy_lib_path = value;
     }},
    {"--android.gradle", nullptr, false,
     [](CliCommand& command, const std::string&) { command.android_gradle = kGradleModeName; }},
};

const OptionSpec* FindOption(const std::string& argument) {
  for (const OptionSpec& option : kOptions) {
    if (argument == option.name)
      return &option;
    if (option.alias != nullptr && argument == option.alias)
      return &option;
  }
  return nullptr;
}

std::string PlatformUsage() {
  return JoinStrings(BackendRegistry::Default().platforms(), ", ");
}

std::string BuildUsage() {
  std::ostringstream out;
  out << "用法: tiecodec [选项] <源文件...>\n\n"
      << "编译选项:\n"
      << "  -o, --output <目录>              设置输出目录\n"
      << "  -p, --package <包名>             设置默认包名\n"
      << "  -d, --define <宏>=<值>           定义编译条件变量\n"
      << "  --dir <目录>                     递归扫描目录下所有源文件\n"
      << "  --source <版本>                  源码文件版本 (" << JoinStrings(kSourceVersions, ", ")
      << "，当前忽略)\n"
      << "  --debug                          调试模式（默认）\n"
      << "  --release                        发布模式\n"
      << "  --hard-mode                      硬输出模式\n"
      << "  --enable-toplevel-stmt           开启顶级语句语法特性支持\n"
      << "  --keep-defaults                  保留默认成员（当前忽略）\n"
      << "  --profile <模式>                 部署模式 (standard, designer)\n"
      << "  --optimize <级别>                优化级别 (" << kMinOptimizeLevel << "-"
      << kMaxOptimizeLevel << "，当前忽略)\n"
      << "  --disable-lint <名称>            禁用指定 lint 检查\n"
      << "  --log-level <级别>               日志级别 (debug, info, warning, error，当前忽略)\n"
      << "  --platform <平台>                目标平台（可用: " << PlatformUsage() << "）\n"
      << "  --friendly-name <模式>           符号名称模式 (random, pinyin, original，默认 random)\n"
      << "  --emit-names <路径>              生成稳定输出名并保存映射表\n"
      << "  --stable-names <路径>            读取稳定名称映射表\n"
      << "  --search-lib-prefix <路径>       增加依赖库搜寻路径\n"
      << "  --search-source-prefix <路径>    增加源文件搜寻路径\n"
      << "  --search-res-prefix <路径>       增加资源搜寻路径\n"
      << "  --search-assets-prefix <路径>    增加资产搜寻路径\n"
      << "  --line-map <路径>                行号表输出路径\n"
      << "  --retrace <行号表> <文件> <行号>  还原源码行号\n"
      << "  --retrace-batch <JSON文件>       批量还原源码行号\n"
      << "  --android.app.config <文件>      指定 App 配置信息文件\n"
      << "  --android.legacy.lib.path <路径> 兼容旧工程依赖目录（当前忽略）\n"
      << "  --android.gradle                 gradle 工程输出格式\n"
      << "  --dump-ast                       解析后打印 AST 摘要\n"
      << "  --check                          仅做解析检查\n"
      << "  -h, --help                       显示帮助\n";
  return out.str();
}

bool IsKnownPlatform(const std::string& platform) {
  const std::vector<std::string>& platforms = BackendRegistry::Default().platforms();
  return std::find(platforms.begin(), platforms.end(), platform) != platforms.end();
}

bool ValidateOptimizeLevel(const std::string& optimize, std::string* error) {
  const bool numeric =
      !optimize.empty() &&
      std::all_of(optimize.begin(), optimize.end(), [](char c) { return c >= '0' && c <= '9'; });
  if (!numeric) {
    *error = "无效的优化级别: " + optimize;
    return false;
  }
  const long level = std::strtol(optimize.c_str(), nullptr, 10);
  if (level < kMinOptimizeLevel || level > kMaxOptimizeLevel) {
    *error = "优化级别必须在 " + std::to_string(kMinOptimizeLevel) + "-" +
             std::to_string(kMaxOptimizeLevel) + " 之间";
    return false;
  }
  return true;
}

}

std::string UsageText() {
  return BuildUsage();
}

bool ParseCommandLine(int argc, char** argv, CliCommand* command, std::string* error) {
  for (int i = 1; i < argc; ++i) {
    const std::string argument = argv[i];
    if (argument == "--retrace") {
      if (i + 3 >= argc) {
        *error = "选项缺少参数: --retrace";
        return false;
      }
      command->retrace_map = argv[++i];
      command->retrace_file = argv[++i];
      command->retrace_line = argv[++i];
      continue;
    }
    const OptionSpec* option = FindOption(argument);
    if (option == nullptr) {
      if (!argument.empty() && argument[0] == '-') {
        *error = "未知选项: " + argument;
        return false;
      }
      command->sources.push_back(argument);
      continue;
    }
    if (!option->takes_value) {
      option->apply(*command, std::string());
      continue;
    }
    if (i + 1 >= argc) {
      *error = argument + " 选项需要参数";
      return false;
    }
    option->apply(*command, argv[++i]);
  }

  if (command->help)
    return true;
  if (!command->retrace_map.empty() || !command->retrace_batch.empty())
    return true;
  if (command->check_only)
    return true;
  if (!command->optimize.empty() && !ValidateOptimizeLevel(command->optimize, error))
    return false;
  if (!IsKnownPlatform(command->platform)) {
    *error = "不支持的目标平台: " + command->platform;
    return false;
  }
  if (command->output_dir.empty()) {
    *error = "未指定输出目录 (-o/--output)";
    return false;
  }
  if (command->sources.empty() && command->source_dirs.empty()) {
    *error = "未指定任何源文件";
    return false;
  }
  return true;
}

}
