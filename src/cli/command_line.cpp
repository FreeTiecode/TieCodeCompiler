#include "command_line.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "backend_registry.h"

namespace tiecodec {
namespace {

std::string PlatformUsage() {
  std::string usage;
  for (const std::string& platform : BackendRegistry::Default().platforms()) {
    if (!usage.empty())
      usage += ", ";
    usage += platform;
  }
  return usage;
}

std::string BuildUsage() {
  const std::string platforms = PlatformUsage();
  std::ostringstream out;
  out << "用法: tiecodec [选项] <源文件...>\n\n"
      << "编译选项:\n"
      << "  -o, --output <目录>              设置输出目录\n"
      << "  -p, --package <包名>             设置默认包名\n"
      << "  -d, --define <宏>=<值>           定义编译条件变量\n"
      << "  --dir <目录>                     递归扫描目录下所有源文件\n"
      << "  --source <版本>                  源码文件版本 (40, 46, 47)\n"
      << "  --debug                          调试模式（默认）\n"
      << "  --release                        发布模式\n"
      << "  --hard-mode                      硬输出模式\n"
      << "  --enable-toplevel-stmt           开启顶级语句语法特性支持\n"
      << "  --profile <模式>                 部署模式 (standard, designer)\n"
      << "  --optimize <级别>                优化级别 (0-3，默认1)\n"
      << "  --disable-lint <名称>            禁用指定 lint 检查\n"
      << "  --log-level <级别>               日志级别 (debug, info, warning, error)\n"
      << "  --platform <平台>                目标平台（可用: " << platforms << "）\n"
      << "  --friendly-name <模式>           符号名称模式 (random, pinyin, original)\n"
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
      << "  --android.legacy.lib.path <路径> 兼容旧工程依赖目录\n"
      << "  --android.gradle                 gradle 工程输出格式\n"
      << "  --dump-ast                       解析后打印 AST 摘要\n"
      << "  --check                          仅做解析检查\n"
      << "  -h, --help                       显示帮助\n";
  return out.str();
}

}  // namespace

std::string UsageText() {
  return BuildUsage();
}

bool ParseCommandLine(int argc, char** argv, CliCommand* command, std::string* error) {
  for (int i = 1; i < argc; ++i) {
    const std::string argument = argv[i];
    const auto value = [&](const char* flag) -> const char* {
      if (i + 1 >= argc) {
        *error = std::string(flag) + " 选项需要参数";
        return nullptr;
      }
      return argv[++i];
    };
    if (argument == "-h" || argument == "--help") {
      command->help = true;
      continue;
    }
    if (argument == "-o" || argument == "--output") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->output_dir = v;
      continue;
    }
    if (argument == "-p" || argument == "--package") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->package_name = v;
      continue;
    }
    if (argument == "--dir") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->source_dirs.emplace_back(v);
      continue;
    }
    if (argument == "--platform") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->platform = v;
      continue;
    }
    if (argument == "--release") {
      command->release = true;
      continue;
    }
    if (argument == "--debug") {
      command->release = false;
      continue;
    }
    if (argument == "--friendly-name") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->friendly_name = v;
      continue;
    }
    if (argument == "--app-name") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->app_name = v;
      continue;
    }
    if (argument == "--version-name") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->version_name = v;
      continue;
    }
    if (argument == "--dump-ast") {
      command->dump_ast = true;
      continue;
    }
    if (argument == "--check") {
      command->check_only = true;
      continue;
    }
    if (argument == "--keep-defaults") {
      command->keep_defaults = true;
      continue;
    }
    if (argument == "-d" || argument == "--define") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      const std::string definition = v;
      const std::size_t equals = definition.find('=');
      if (equals == std::string::npos) {
        command->defines.emplace_back(definition, "真");
      } else {
        command->defines.emplace_back(definition.substr(0, equals), definition.substr(equals + 1));
      }
      continue;
    }
    if (argument == "--source") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->source_version = v;
      continue;
    }
    if (argument == "--hard-mode") {
      command->hard_mode = true;
      continue;
    }
    if (argument == "--enable-toplevel-stmt") {
      command->enable_toplevel_stmt = true;
      continue;
    }
    if (argument == "--profile") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->profile = v;
      continue;
    }
    if (argument == "--optimize") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->optimize = v;
      continue;
    }
    if (argument == "--disable-lint") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->disable_lint.emplace_back(v);
      continue;
    }
    if (argument == "--log-level") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->log_level = v;
      continue;
    }
    if (argument == "--emit-names") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->emit_names = v;
      continue;
    }
    if (argument == "--stable-names") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->stable_names = v;
      continue;
    }
    if (argument == "--search-lib-prefix" || argument == "--search-source-prefix" ||
        argument == "--search-res-prefix" || argument == "--search-assets-prefix") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      if (argument == "--search-lib-prefix")
        command->search_lib_prefixes.emplace_back(v);
      else if (argument == "--search-source-prefix")
        command->search_source_prefixes.emplace_back(v);
      else if (argument == "--search-res-prefix")
        command->search_res_prefixes.emplace_back(v);
      else
        command->search_assets_prefixes.emplace_back(v);
      continue;
    }
    if (argument == "--line-map") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->line_map = v;
      continue;
    }
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
    if (argument == "--retrace-batch") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->retrace_batch = v;
      continue;
    }
    if (argument == "--android.app.config") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->android_app_config = v;
      continue;
    }
    if (argument == "--android.legacy.lib.path") {
      const char* v = value(argument.c_str());
      if (v == nullptr)
        return false;
      command->android_legacy_lib_path = v;
      continue;
    }
    if (argument == "--android.gradle") {
      command->android_gradle = "gradle";
      continue;
    }
    if (!argument.empty() && argument[0] == '-') {
      *error = "未知选项: " + argument;
      return false;
    }
    command->sources.push_back(argument);
  }
  if (command->help)
    return true;
  if (!command->retrace_map.empty() || !command->retrace_batch.empty())
    return true;
  if (command->check_only)
    return true;
  if (!command->optimize.empty()) {
    const std::string& optimize = command->optimize;
    const bool numeric =
        !optimize.empty() &&
        std::all_of(optimize.begin(), optimize.end(), [](char c) { return c >= '0' && c <= '9'; });
    if (!numeric) {
      *error = "无效的优化级别: " + optimize;
      return false;
    }
    const long level = std::strtol(optimize.c_str(), nullptr, 10);
    if (level < 0 || level > 3) {
      *error = "优化级别必须在 0-3 之间";
      return false;
    }
  }
  {
    static const char* kPlatforms[] = {"android", "harmony", "linux", "windows", "ios", "html"};
    bool known_platform = false;
    for (const char* platform : kPlatforms) {
      if (command->platform == platform)
        known_platform = true;
    }
    if (!known_platform) {
      *error = "不支持的目标平台: " + command->platform;
      return false;
    }
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

}  // namespace tiecodec
