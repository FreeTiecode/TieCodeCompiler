#ifndef TIECODEC_ANALYSIS_LOCAL_USE_H
#define TIECODEC_ANALYSIS_LOCAL_USE_H

#include <set>
#include <string>

#include "ast.h"

namespace tiecodec {

/// Names of local variables of `member` that are referenced anywhere outside
/// their own declaration. Unreferenced locals are dropped by the reference
/// compiler together with their initializer.
std::set<std::string> UsedLocalNames(const Member& member);

/// True when a variable declaration statement still has a live consumer.
bool IsLocalDeclarationUsed(const Stmt& statement, const std::set<std::string>& used);

}  // namespace tiecodec

#endif  // TIECODEC_ANALYSIS_LOCAL_USE_H
