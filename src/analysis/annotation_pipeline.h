#ifndef TIECODEC_ANALYSIS_ANNOTATION_PIPELINE_H
#define TIECODEC_ANALYSIS_ANNOTATION_PIPELINE_H

#include <map>
#include <vector>

#include "annotation_index.h"
#include "annotation_registry.h"
#include "ast.h"
#include "effect_store.h"
#include "plan_hints.h"
#include "semantic_model.h"
#include "symbols.h"
#include "target_key.h"

namespace tiecodec {

void RunCompileConditions(std::vector<Unit>& units, const std::map<std::string, bool>& defines);

void DeclareAnnotations(const std::vector<Unit>& units, const AnnotationRegistry& registry,
                        const TargetKey& target, const AnnotationSymbolIndex& index,
                        SemanticModel& semantics, Diagnostics& diagnostics);

void ApplyAnnotations(const std::vector<Unit>& units, const SymbolTable& symbols,
                      const AnnotationRegistry& registry, const TargetKey& target,
                      const SemanticModel& semantics, EffectStore& effects, PlanHints& plan,
                      Diagnostics& diagnostics);

}

#endif
