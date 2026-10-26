#ifndef TIECODEC_TARGETS_API_TARGET_H
#define TIECODEC_TARGETS_API_TARGET_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "effect_store.h"
#include "output_plan.h"
#include "output_sink.h"
#include "semantic_model.h"
#include "symbols.h"
#include "target_config.h"
#include "target_key.h"

namespace tiecodec {

struct Diagnostics;

struct LineRef {
  std::string file;
  int line = 0;
};

struct GeneratedSource {
  std::string file_name;
  std::string content;
  std::vector<LineRef> line_refs;
  const ClassInfo* klass = nullptr;
  std::string directory;
};

struct GeneratedProgram {
  std::vector<GeneratedSource> sources;
  std::map<const ClassInfo*, std::string> qualified_names;
  std::vector<std::pair<std::string, std::string>> mapping_names;
  std::string entry_qualified_name;
  std::string entry_class_name;
};

struct GenerationInput {
  std::vector<Unit>& units;
  const SymbolTable& symbols;
  const SemanticModel& semantics;
  const EffectStore& effects;
  const OutputPlan& plan;
  const TargetConfig& config;
  TargetKey target;
  const ClassInfo* entry = nullptr;
};

class PlatformExtension {
public:
  virtual ~PlatformExtension() = default;
};

class LanguageBackend {
public:
  virtual ~LanguageBackend() = default;

  virtual TargetKey Key() const = 0;
  virtual GeneratedProgram Emit(const GenerationInput& input, Diagnostics& diagnostics) = 0;
};

class PlatformBackend {
public:
  virtual ~PlatformBackend() = default;

  virtual TargetKey Key() const = 0;
  virtual bool Generate(const GenerationInput& input, LanguageBackend& language,
                        Diagnostics& diagnostics, OutputSink& sink) = 0;
};

struct TargetSession {
  std::shared_ptr<PlatformExtension> extension;
  std::unique_ptr<LanguageBackend> language;
  std::unique_ptr<PlatformBackend> platform;
};

struct TargetDescriptor {
  TargetKey key;
  std::string display_name;
  std::string entry_class_name;
  std::string entry_method;
  std::function<TargetSession(const GenerationInput&)> create;
};

}

#endif
