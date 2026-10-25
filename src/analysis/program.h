#ifndef TIECODEC_ANALYSIS_PROGRAM_H
#define TIECODEC_ANALYSIS_PROGRAM_H

#include <string>
#include <vector>

#include "ast.h"
#include "semantic_model.h"
#include "symbols.h"

namespace tiecodec {

struct Program {
  std::vector<Unit> units;
  SymbolTable symbols;

  void BuildSymbols(const SemanticModel& semantics, Diagnostics* diagnostics);
};

const ClassInfo* FindEntryClass(const SymbolTable& symbols,
                                const std::string& name = kEntryClassName);

bool ValidateSymbolPointers(const Program& program, Diagnostics* diagnostics);

}

#endif
