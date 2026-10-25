#ifndef TIECODEC_CLI_COMPILATION_SESSION_H
#define TIECODEC_CLI_COMPILATION_SESSION_H

#include <map>
#include <string>

#include "annotation_context.h"
#include "console_reporter.h"
#include "diagnostics.h"
#include "effect_store.h"
#include "extension_registry.h"
#include "plan_hints.h"
#include "program.h"
#include "semantic_model.h"
#include "target.h"
#include "target_config.h"
#include "target_key.h"
#include "target_registry.h"

namespace tiecodec {

struct CliCommand;

class CompilationSession {
public:
  CompilationSession(const CliCommand& command, const TargetRegistry& targets);

  int Run();

private:
  void RunAnnotationPhase();
  int RunCheck();
  int PrepareTarget();
  bool RunPlanAndGenerate();

  const CliCommand& command_;
  const TargetRegistry& targets_;
  Diagnostics diagnostics_;
  ConsoleReporter reporter_;
  ExtensionRegistry extensions_;
  TargetKey target_;
  ValidationEnvironment validation_environment_;
  std::map<std::string, bool> defines_;
  Program program_;
  SemanticModel semantics_;
  EffectStore effects_;
  PlanHints plan_hints_;
  TargetConfig config_;
  const TargetDescriptor* target_descriptor_ = nullptr;
  const ClassInfo* entry_ = nullptr;
  std::string error_;
  bool startup_failed_ = false;
};

}

#endif
