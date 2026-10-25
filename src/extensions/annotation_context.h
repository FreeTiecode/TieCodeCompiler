#ifndef TIECODEC_EXTENSIONS_ANNOTATION_CONTEXT_H
#define TIECODEC_EXTENSIONS_ANNOTATION_CONTEXT_H

#include <map>
#include <string>
#include <vector>

#include "annotation_index.h"
#include "ast.h"
#include "diagnostics.h"
#include "effect_store.h"
#include "plan_hints.h"
#include "semantic_model.h"
#include "symbols.h"
#include "target_key.h"

namespace tiecodec {

struct ValidationEnvironment {
  std::vector<std::string> source_prefixes;
  std::vector<std::string> res_prefixes;
  std::vector<std::string> lib_prefixes;
  std::vector<std::string> assets_prefixes;
  std::map<std::string, bool> defines;
};

struct ValidationContext {
  const TargetKey& target;
  const AnnotationSymbolIndex& index;
  const ValidationEnvironment& environment;
  Diagnostics& diagnostics;
  const Unit& unit;
  const ClassDecl* klass = nullptr;
  const Member* member = nullptr;
  std::string owner_class;
  bool type_error = false;
  bool had_error = false;
};

struct DeclareContext {
  const TargetKey& target;
  const AnnotationSymbolIndex& index;
  const Unit& unit;
  SemanticModel& semantics;
  Diagnostics& diagnostics;
};

struct ApplyContext {
  const TargetKey& target;
  const SymbolTable& symbols;
  const SemanticModel& semantics;
  EffectStore& effects;
  PlanHints& plan;
  Diagnostics& diagnostics;
};

}

#endif
