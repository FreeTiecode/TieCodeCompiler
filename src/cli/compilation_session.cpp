#include "compilation_session.h"

#include <iostream>
#include <set>
#include <vector>

#include "annotation_pipeline.h"
#include "annotation_validation.h"
#include "ast_dump.h"
#include "command_line.h"
#include "constants.h"
#include "output_sink.h"
#include "register_extensions.h"
#include "requirement_validation.h"
#include "source_loader.h"
#include "symbol_resolution.h"
#include "target_config_assembler.h"
#include "usage_analysis.h"

namespace tiecodec {

CompilationSession::CompilationSession(const CliCommand& command, const TargetRegistry& targets)
    : command_(command), targets_(targets), target_{command.platform, command.language} {
  if (!RegisterBuiltinExtensions(extensions_, &error_))
    startup_failed_ = true;
  validation_environment_.source_prefixes = command.search_source_prefixes;
  validation_environment_.res_prefixes = command.search_res_prefixes;
  validation_environment_.lib_prefixes = command.search_lib_prefixes;
  validation_environment_.assets_prefixes = command.search_assets_prefixes;
  defines_[kDefineDebugBuild] = !command.release;
  defines_[kDefineReleaseBuild] = command.release;
  for (const auto& definition : command.defines)
    defines_[definition.first] = IsTruthyLiteral(definition.second);
  validation_environment_.defines = defines_;
}

int CompilationSession::Run() {
  if (startup_failed_) {
    std::cerr << "错误: " << error_ << "\n";
    return 2;
  }
  const SourceLoadStatus load_status =
      LoadProgramSources(command_, &program_, &diagnostics_, &error_);
  if (load_status == SourceLoadStatus::kScanError) {
    std::cerr << "[ERROR] " << error_ << "\n";
    return 2;
  }
  if (load_status == SourceLoadStatus::kNoSources) {
    std::cerr << "错误: 没有待编译的源文件\n";
    return 2;
  }

  RunAnnotationPhase();
  if (command_.check_only)
    return RunCheck();

  const int prepare_status = PrepareTarget();
  if (prepare_status != 0)
    return prepare_status;

  bool ok = !diagnostics_.has_errors();
  if (ok && target_descriptor_ == nullptr) {
    std::cerr << "错误: 暂不支持的目标平台或语言: " << command_.platform << "/" << command_.language
              << "\n";
    return 1;
  }
  if (ok)
    ok = RunPlanAndGenerate();
  return reporter_.ReportCompile(diagnostics_, ok);
}

int CompilationSession::RunCheck() {
  const int prepare_status = PrepareTarget();
  if (prepare_status != 0)
    return prepare_status;
  if (target_descriptor_ == nullptr) {
    diagnostics_.ErrorGlobal(
        "暂不支持的目标平台或语言: " + command_.platform + "/" + command_.language,
        "target.unsupported");
    return reporter_.ReportCheck(diagnostics_, program_.units.size());
  }
  ApplyAnnotations(program_.units, program_.symbols, extensions_.annotations(), target_, semantics_,
                   effects_, plan_hints_, diagnostics_);
  ApplyRequirements(extensions_.requirements(), target_, program_.symbols, semantics_, plan_hints_);
  if (diagnostics_.has_errors())
    return reporter_.ReportCheck(diagnostics_, program_.units.size());
  const OutputPlan plan = ComputeOutputPlan(program_.symbols, entry_, semantics_, plan_hints_,
                                            config_.profile == "designer");
  GenerationInput input{program_.units, program_.symbols,        semantics_, effects_, plan,
                        config_,        target_descriptor_->key, entry_};
  TargetSession session = target_descriptor_->create(input);
  if (session.language != nullptr)
    (void)session.language->Emit(input, diagnostics_);
  return reporter_.ReportCheck(diagnostics_, program_.units.size());
}

void CompilationSession::RunAnnotationPhase() {
  AnnotationSymbolIndex annotation_index;
  BuildAnnotationSymbolIndex(program_.units, &annotation_index);
  std::set<const Annotation*> invalid_annotations;
  for (const Unit& unit : program_.units) {
    ValidateAnnotations(unit, annotation_index, extensions_.annotations(), target_,
                        validation_environment_, &diagnostics_, &invalid_annotations);
    ValidateAnnotationFiles(unit, extensions_.annotations(), target_, validation_environment_,
                            &diagnostics_, &invalid_annotations);
  }
  RunCompileConditions(program_.units, defines_);
  if (command_.dump_ast) {
    for (const Unit& unit : program_.units)
      DumpAst(unit);
  }
  DeclareAnnotations(program_.units, extensions_.annotations(), target_, annotation_index,
                     semantics_, diagnostics_);
  ValidateForbiddenCreation(program_.units, semantics_, &diagnostics_);
}

int CompilationSession::PrepareTarget() {
  program_.BuildSymbols(semantics_, &diagnostics_);
  if (!ValidateSymbolPointers(program_, &diagnostics_))
    return 0;
  ValidateRequirements(extensions_.requirements(), target_, program_.symbols, diagnostics_);
  target_descriptor_ = targets_.Find(command_.platform, command_.language);
  const std::string entry_name = target_descriptor_ == nullptr
                                     ? std::string(kEntryClassName)
                                     : target_descriptor_->entry_class_name;
  entry_ = FindEntryClass(program_.symbols, entry_name);
  if (!BuildTargetConfig(command_, &config_, &error_)) {
    std::cerr << "[ERROR] " << error_ << "\n";
    return 2;
  }
  return 0;
}

bool CompilationSession::RunPlanAndGenerate() {
  ApplyAnnotations(program_.units, program_.symbols, extensions_.annotations(), target_, semantics_,
                   effects_, plan_hints_, diagnostics_);
  ApplyRequirements(extensions_.requirements(), target_, program_.symbols, semantics_, plan_hints_);
  if (entry_ != nullptr && !target_descriptor_->entry_method.empty()) {
    const ClassInfo* method_owner = nullptr;
    const Member* entry_method = FindMemberInChain(program_.symbols, *entry_,
                                                   target_descriptor_->entry_method, &method_owner);
    if (entry_method == nullptr) {
      diagnostics_.Fatal("入口类 \"" + entry_->qualified_name + "\" 缺少方法 \"" +
                             target_descriptor_->entry_method + "\"",
                         "output.entry.method-not-found");
    } else {
      plan_hints_.ForceMember(method_owner == nullptr ? entry_ : method_owner, entry_method);
    }
  }
  if (diagnostics_.has_errors())
    return false;

  const OutputPlan plan = ComputeOutputPlan(program_.symbols, entry_, semantics_, plan_hints_,
                                            config_.profile == "designer");
  GenerationInput input{program_.units, program_.symbols,        semantics_, effects_, plan,
                        config_,        target_descriptor_->key, entry_};
  TargetSession session = target_descriptor_->create(input);
  FileSystemOutputSink sink(config_.output_dir);
  std::cout << "开始编译...\n";
  std::cout << "目标平台: " << target_descriptor_->display_name << "\n";
  std::cout << "输出目录: " << command_.output_dir << "\n";
  if (session.platform == nullptr || session.language == nullptr) {
    diagnostics_.Fatal("目标后端初始化失败", "target.create");
    return false;
  }
  const bool generated = session.platform->Generate(input, *session.language, diagnostics_, sink);
  return ValidateSymbolPointers(program_, &diagnostics_) && generated;
}

}
