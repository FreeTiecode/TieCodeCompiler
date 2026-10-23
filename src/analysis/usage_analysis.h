#ifndef TIECODEC_ANALYSIS_USAGE_ANALYSIS_H
#define TIECODEC_ANALYSIS_USAGE_ANALYSIS_H

#include <set>
#include <utility>
#include <vector>

#include "ast.h"
#include "environment_contract.h"
#include "symbols.h"

namespace tiecodec {

struct OutputPlan {
  std::vector<const ClassInfo*> classes;
  std::set<const Member*> members;

  std::vector<std::pair<const ClassInfo*, const Member*>> virtual_overrides;
};

std::set<const Member*> ComputeUsedMembers(
    const SymbolTable& symbols, const EnvironmentContract& contract, const ClassInfo* entry,
    const std::vector<const ClassInfo*>& emitted_classes,
    std::vector<std::pair<const ClassInfo*, const Member*>>* virtual_overrides = nullptr);

OutputPlan ComputeOutputPlan(const SymbolTable& symbols, const EnvironmentContract& contract,
                             const ClassInfo* entry);

}  // namespace tiecodec

#endif
