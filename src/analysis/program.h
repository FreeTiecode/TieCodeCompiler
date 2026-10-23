#ifndef TIECODEC_ANALYSIS_PROGRAM_H
#define TIECODEC_ANALYSIS_PROGRAM_H

#include <string>
#include <vector>

#include "ast.h"
#include "symbols.h"

namespace tiecodec {

struct Program {
  std::vector<Unit> units;
  SymbolTable symbols;

  void BuildSymbols(Diagnostics* diagnostics);
};

const ClassInfo* FindEntryClass(const SymbolTable& symbols,
                                const std::string& name = kEntryClassName);

}  // namespace tiecodec

#endif
