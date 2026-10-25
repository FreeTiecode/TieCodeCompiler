#include "target_config_assembler.h"

#include <vector>

#include "app_config.h"
#include "command_line.h"
#include "options.h"
#include "target_config.h"
#include "text_util.h"

namespace tiecodec {
namespace {

void SetListOption(TargetConfig* config, const char* key, const std::vector<std::string>& values) {
  if (!values.empty())
    config->options[key] = JoinStrings(values, ";");
}

}

bool BuildTargetConfig(const CliCommand& command, TargetConfig* config, std::string* error) {
  config->output_dir = command.output_dir;
  config->friendly_name = command.friendly_name;
  config->profile = command.profile;
  if (!command.package_name.empty()) {
    config->package_name = command.package_name;
    config->namespace_name = command.package_name;
  }
  if (!command.app_name.empty())
    config->app_name = command.app_name;
  if (!command.version_name.empty())
    config->version_name = command.version_name;
  if (!command.android_app_config.empty() &&
      !LoadAppConfig(command.android_app_config, config, error)) {
    return false;
  }
  if (!command.disable_lint.empty())
    config->options[option::kDisableLint] = JoinStrings(command.disable_lint, ",");
  config->options[option::kDefine] = std::to_string(command.defines.size());
  config->options[option::kSource] = command.source_version;
  config->options[option::kProfile] = command.profile;
  config->options[option::kOptimize] = command.optimize;
  config->options[option::kLogLevel] = command.log_level;
  config->options[option::kHardMode] = command.hard_mode ? "1" : "0";
  config->options[option::kRelease] = command.release ? "1" : "0";
  config->options[option::kGradle] = command.android_gradle.empty() ? "" : "1";
  config->options[option::kLegacyLibPath] = command.android_legacy_lib_path;
  config->options[option::kEmitNames] = command.emit_names;
  config->options[option::kStableNames] = command.stable_names;
  config->options[option::kLineMap] = command.line_map;
  config->options[option::kEnableTopLevelStatements] = command.enable_toplevel_stmt ? "1" : "0";
  config->options[option::kDisableFtcFeatures] = command.disable_ftc_features ? "1" : "0";
  SetListOption(config, option::kSearchLibPrefix, command.search_lib_prefixes);
  SetListOption(config, option::kSearchSourcePrefix, command.search_source_prefixes);
  SetListOption(config, option::kSearchResPrefix, command.search_res_prefixes);
  SetListOption(config, option::kSearchAssetsPrefix, command.search_assets_prefixes);
  return true;
}

}
