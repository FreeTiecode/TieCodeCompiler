#ifndef TIECODEC_ANALYSIS_LOCAL_USE_H
#define TIECODEC_ANALYSIS_LOCAL_USE_H

#include <set>
#include <string>

#include "ast.h"

namespace tiecodec {

std::set<std::string> UsedLocalNames(const Member& member);

bool IsLocalDeclarationUsed(const Stmt& statement, const std::set<std::string>& used);

}

#endif
