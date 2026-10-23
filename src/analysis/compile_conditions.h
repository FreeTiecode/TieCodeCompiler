#ifndef TIECODEC_ANALYSIS_COMPILE_CONDITIONS_H
#define TIECODEC_ANALYSIS_COMPILE_CONDITIONS_H

#include <map>
#include <string>

#include "ast.h"

namespace tiecodec {

/// Evaluate a @编译条件 expression (`已定义("X")`, `且`, `或`, `非`, 真/假,
/// define names). Returns true for an empty expression.
bool EvaluateCompileCondition(const std::string& expression,
                              const std::map<std::string, bool>& defines);

/// Drop classes and members whose @编译条件 evaluates to false.
void ApplyCompileConditions(Unit* unit, const std::map<std::string, bool>& defines);

}  // namespace tiecodec

#endif  // TIECODEC_ANALYSIS_COMPILE_CONDITIONS_H
