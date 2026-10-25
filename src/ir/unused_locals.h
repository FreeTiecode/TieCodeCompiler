#ifndef TIECODEC_ANALYSIS_UNUSED_LOCALS_H
#define TIECODEC_ANALYSIS_UNUSED_LOCALS_H

#include "ast.h"
#include "diagnostics.h"

namespace tiecodec {

void RemoveUnusedLocals(ClassDecl* class_decl, const std::string& file, Diagnostics* diagnostics);

void RemoveUnusedLocals(Unit* unit, Diagnostics* diagnostics);

}

#endif
