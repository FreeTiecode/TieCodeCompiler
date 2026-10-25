#ifndef TIECODEC_ANALYSIS_REQUIREMENT_VALIDATION_H
#define TIECODEC_ANALYSIS_REQUIREMENT_VALIDATION_H

#include "plan_hints.h"
#include "requirement_registry.h"
#include "semantic_model.h"
#include "symbols.h"
#include "target_key.h"

namespace tiecodec {

void ValidateRequirements(const RequirementRegistry& registry, const TargetKey& target,
                          const SymbolTable& symbols, Diagnostics& diagnostics);

void ApplyRequirements(const RequirementRegistry& registry, const TargetKey& target,
                       const SymbolTable& symbols, const SemanticModel& semantics, PlanHints& plan);

}

#endif
