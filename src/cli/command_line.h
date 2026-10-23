#ifndef TIECODEC_CLI_COMMAND_LINE_H
#define TIECODEC_CLI_COMMAND_LINE_H

#include <string>
#include <utility>
#include <vector>

namespace tiecodec {

struct CliCommand {
  std::vector<std::string> source_dirs;
  std::vector<std::string> sources;
  std::string output_dir;
  std::string package_name;
  std::string app_name;
  std::string version_name;
  std::string friendly_name = "random";
  std::string platform = "android";
  bool release = false;
  bool help = false;
  bool dump_ast = false;
  bool check_only = false;
  bool keep_defaults = false;
  std::vector<std::pair<std::string, std::string>> defines;
  std::vector<std::string> disable_lint;
  std::string source_version;
  std::string profile = "standard";
  std::string optimize = "1";
  std::string log_level = "info";
  std::string emit_names;
  std::string stable_names;
  std::vector<std::string> search_lib_prefixes;
  std::vector<std::string> search_source_prefixes;
  std::vector<std::string> search_res_prefixes;
  std::vector<std::string> search_assets_prefixes;
  std::string line_map;
  std::string retrace_map;
  std::string retrace_file;
  std::string retrace_line;
  std::string retrace_batch;
  std::string android_app_config;
  std::string android_legacy_lib_path;
  std::string android_gradle;
  bool hard_mode = false;
  bool enable_toplevel_stmt = false;
};

bool ParseCommandLine(int argc, char** argv, CliCommand* command, std::string* error);

std::string UsageText();

}  // namespace tiecodec

#endif
