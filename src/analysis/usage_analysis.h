#ifndef TIECODEC_ANALYSIS_USAGE_ANALYSIS_H
#define TIECODEC_ANALYSIS_USAGE_ANALYSIS_H

#include <set>
#include <utility>
#include <vector>

#include "ast.h"
#include "output_plan.h"
#include "plan_hints.h"
#include "semantic_model.h"
#include "symbols.h"

namespace tiecodec {

std::set<const Member*> ComputeUsedMembers(
    const SymbolTable& symbols, const ClassInfo* entry,
    const std::vector<const ClassInfo*>& emitted_classes, const SemanticModel& semantics,
    const PlanHints& hints,
    std::vector<std::pair<const ClassInfo*, const Member*>>* virtual_overrides = nullptr);

OutputPlan ComputeOutputPlan(const SymbolTable& symbols, const ClassInfo* entry,
                             const SemanticModel& semantics, const PlanHints& hints,
                             bool designer_profile = false);

}

#endif
