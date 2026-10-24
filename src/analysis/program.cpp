#include "program.h"

namespace tiecodec {

void Program::BuildSymbols(Diagnostics* diagnostics) {
  for (const Unit& unit : units)
    symbols.AddUnit(unit, diagnostics);
}

const ClassInfo* FindEntryClass(const SymbolTable& symbols, const std::string& name) {
  for (const std::string& qualified_name : symbols.class_names()) {
    const std::size_t dot = qualified_name.rfind('.');
    const std::string simple =
        dot == std::string::npos ? qualified_name : qualified_name.substr(dot + 1);
    if (simple != name)
      continue;
    return symbols.Find(qualified_name);
  }
  return nullptr;
}

}
