#ifndef TIECODEC_IR_ASYNC_ANALYSIS_H
#define TIECODEC_IR_ASYNC_ANALYSIS_H

#include "ast.h"
#include "semantic_model.h"

namespace tiecodec {

bool AsyncMemberUsesGlobalScheduler(const Member& member, const SemanticModel& semantics);

}

#endif
