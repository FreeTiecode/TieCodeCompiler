#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "annotation_rules.h"
#include "app_config.h"
#include "ast.h"
#include "backend.h"
#include "backend_registry.h"
#include "command_line.h"
#include "compile_conditions.h"
#include "diagnostics.h"
#include "parser.h"
#include "program.h"
#include "retrace.h"
#include "source.h"

using namespace tiecodec;

namespace {

void DumpAst(const Unit& unit) {
  std::cout << "文件: " << unit.file << " 包名: " << unit.package_name << "\n";
  for (const ClassDecl& class_decl : unit.classes) {
    std::cout << "  类 " << class_decl.name;
    if (!class_decl.base_name.empty())
      std::cout << " : " << class_decl.base_name;
    std::cout << " (" << class_decl.members.size() << " 个成员)\n";
    for (const Member& member : class_decl.members) {
      const char* kind = "成员";
      switch (member.kind) {
        case MemberKind::kVariable:
          kind = "变量";
          break;
        case MemberKind::kConstant:
          kind = "常量";
          break;
        case MemberKind::kMethod:
          kind = "方法";
          break;
        case MemberKind::kPropertyRead:
          kind = "属性读";
          break;
        case MemberKind::kPropertyWrite:
          kind = "属性写";
          break;
        case MemberKind::kEventDefinition:
          kind = "定义事件";
          break;
        case MemberKind::kEventBlock:
          kind = "事件块";
          break;
        case MemberKind::kCodeBlock:
          kind = "代码块";
          break;
      }
      std::cout << "    " << kind << " " << member.name;
      if (!member.type_name.empty())
        std::cout << ": " << member.type_name;
      std::cout << " 语句=" << member.body.size();
      if (!member.code_text.empty())
        std::cout << " 内嵌代码";
      std::cout << "\n";
    }
  }
}

}  // namespace

int main(int argc, char** argv) {
  CliCommand command;
  std::string error;
  if (!ParseCommandLine(argc, argv, &command, &error)) {
    std::cerr << "错误: " << error << "\n";
    return 1;
  }
  if (command.friendly_name != "original" && command.friendly_name != "pinyin" &&
      command.friendly_name != "random") {
    command.friendly_name = "random";
  }
  if (command.help) {
    std::cout << UsageText();
    return 0;
  }
  if (!command.retrace_batch.empty()) {
    return RunRetraceBatch(command.retrace_batch);
  }
  if (!command.retrace_map.empty()) {
    RetraceRequest request;
    request.mapping_path = command.retrace_map;
    request.file = command.retrace_file;
    request.line = command.retrace_line;
    return RunRetrace(request);
  }

  FileSystemSourceProvider source_provider;
  std::vector<std::string> files;
  for (const std::string& directory : command.source_dirs) {
    if (!source_provider.ScanDirectory(directory, &files, &error)) {
      std::cerr << "[ERROR] " << error << "\n";
      return 2;
    }
  }
  files.insert(files.end(), command.sources.begin(), command.sources.end());

  Diagnostics diagnostics;
  ConsoleDiagnosticReporter reporter;
  Program program;
  program.units.reserve(files.size());

  std::map<std::string, bool> defines;
  defines["调试版"] = !command.release;
  defines["正式版"] = command.release;
  for (const auto& definition : command.defines) {
    const std::string& value = definition.second;
    defines[definition.first] = value != "假" && value != "false" && value != "0";
  }

  for (const std::string& file : files) {
    std::cout << "添加源文件: " << file << "\n";
    SourceFile source_file;
    if (!source_provider.ReadFile(file, &source_file, &error)) {
      diagnostics.Fatal(error, "io.read");
      continue;
    }
    Unit unit;
    unit.file = file;
    Parser parser(std::move(source_file.content), file, &diagnostics);
    parser.SetTopLevelMode(command.enable_toplevel_stmt);
    parser.ParseUnit(&unit);
    ValidateAnnotations(unit, &diagnostics);
    ValidateAnnotationFiles(unit, command.search_source_prefixes, command.search_res_prefixes,
                            command.search_lib_prefixes, command.search_assets_prefixes,
                            &diagnostics);
    ApplyCompileConditions(&unit, defines);
    if (command.dump_ast)
      DumpAst(unit);
    program.units.push_back(std::move(unit));
  }

  if (command.check_only) {
    diagnostics.Flush(reporter);
    if (diagnostics.error_count() > 0) {
      std::cerr << "解析失败，存在 " << diagnostics.error_count() << " 个错误\n";
      return 1;
    }
    std::cout << "解析通过，共 " << program.units.size() << " 个源文件\n";
    return 0;
  }

  program.BuildSymbols(&diagnostics);
  const ClassInfo* entry = FindEntryClass(program.symbols);

  BackendConfig config;
  config.output_dir = command.output_dir;
  config.friendly_name = command.friendly_name;
  if (!command.package_name.empty()) {
    config.package_name = command.package_name;
    config.namespace_name = command.package_name;
  }
  if (!command.app_name.empty())
    config.app_name = command.app_name;
  if (!command.version_name.empty())
    config.version_name = command.version_name;
  if (!command.android_app_config.empty()) {
    if (!LoadAppConfig(command.android_app_config, &config, &error)) {
      std::cerr << "[ERROR] " << error << "\n";
      return 2;
    }
  }
  if (!command.disable_lint.empty()) {
    std::string lint;
    for (const std::string& name : command.disable_lint) {
      if (!lint.empty())
        lint += ",";
      lint += name;
    }
    config.options["disable-lint"] = lint;
  }
  config.options["define"] = std::to_string(command.defines.size());
  config.options["source"] = command.source_version;
  config.options["profile"] = command.profile;
  config.options["optimize"] = command.optimize;
  config.options["log-level"] = command.log_level;
  config.options["hard-mode"] = command.hard_mode ? "1" : "0";
  config.options["release"] = command.release ? "1" : "0";
  config.options["android.gradle"] = command.android_gradle.empty() ? "" : "1";
  config.options["android.legacy.lib.path"] = command.android_legacy_lib_path;
  config.options["emit-names"] = command.emit_names;
  config.options["stable-names"] = command.stable_names;
  config.options["line-map"] = command.line_map;
  config.options["enable-toplevel-stmt"] = command.enable_toplevel_stmt ? "1" : "0";
  for (const std::string& prefix : command.search_lib_prefixes) {
    config.options["search-lib-prefix"] += prefix + ";";
  }
  for (const std::string& prefix : command.search_source_prefixes) {
    config.options["search-source-prefix"] += prefix + ";";
  }
  for (const std::string& prefix : command.search_res_prefixes) {
    config.options["search-res-prefix"] += prefix + ";";
  }
  for (const std::string& prefix : command.search_assets_prefixes) {
    config.options["search-assets-prefix"] += prefix + ";";
  }

  bool ok = !diagnostics.has_errors();
  std::unique_ptr<Backend> backend;
  if (ok) {
    backend = CreateBackend(command.platform, program.symbols);
    if (backend == nullptr) {
      std::cerr << "错误: 暂不支持的目标平台: " << command.platform << "\n";
      return 1;
    }
    std::cout << "开始编译...\n";
    std::cout << "目标平台: " << backend->DisplayName() << "\n";
    std::cout << "输出目录: " << command.output_dir << "\n";
    ok = backend->Generate(entry, config, diagnostics);
  }

  diagnostics.Flush(reporter);

  if (ok) {
    if (diagnostics.warning_count() > 0) {
      std::cout << "编译成功，但存在 " << diagnostics.warning_count() << " 个警告\n";
    } else {
      std::cout << "编译完成\n";
    }
    return 0;
  }
  if (diagnostics.fatal_count() > 0) {
    std::cout << "编译失败，存在致命错误\n";
  } else {
    std::cout << "编译失败，存在 " << diagnostics.error_count() << " 个错误\n";
  }
  return 1;
}
