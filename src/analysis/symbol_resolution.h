#ifndef TIECODEC_ANALYSIS_SYMBOL_RESOLUTION_H
#define TIECODEC_ANALYSIS_SYMBOL_RESOLUTION_H

#include <string>

#include "symbols.h"

namespace tiecodec {

const Member* FindMemberInChain(const SymbolTable& symbols, const ClassInfo& klass,
                                const std::string& name, const ClassInfo** owner);

const Member* ResolveGlobalMethod(const SymbolTable& symbols, const ClassInfo& context,
                                  const std::string& name, const ClassInfo** owner);

/// Looks up an implicitly referenced member of a @全局类 class.
const Member* FindGlobalClassMember(const SymbolTable& symbols, const ClassInfo& context,
                                    const std::string& name, const ClassInfo** owner);

}  // namespace tiecodec

#endif
