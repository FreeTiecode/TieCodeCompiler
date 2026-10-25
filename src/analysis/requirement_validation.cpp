#include "requirement_validation.h"

#include "requirement.h"

namespace tiecodec {

void ValidateRequirements(const RequirementRegistry& registry, const TargetKey& target,
                          const SymbolTable& symbols, Diagnostics& diagnostics) {
  for (const PlatformRequirement* requirement : registry.Matching(target))
    requirement->Validate(symbols, diagnostics);
}

void ApplyRequirements(const RequirementRegistry& registry, const TargetKey& target,
                       const SymbolTable& symbols, const SemanticModel& semantics,
                       PlanHints& plan) {
  for (const PlatformRequirement* requirement : registry.Matching(target))
    requirement->Apply(symbols, semantics, plan);
}

}
