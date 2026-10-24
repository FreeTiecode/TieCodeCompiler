#ifndef TIECODEC_ANALYSIS_COMPILE_CONDITIONS_H
#define TIECODEC_ANALYSIS_COMPILE_CONDITIONS_H

#include <map>
#include <string>

#include "ast.h"

namespace tiecodec {

bool EvaluateCompileCondition(const std::string& expression,
                              const std::map<std::string, bool>& defines);

bool CompileConditionParses(const std::string& expression,
                            const std::map<std::string, bool>& defines);

void ApplyCompileConditions(Unit* unit, const std::map<std::string, bool>& defines);

}

#endif
