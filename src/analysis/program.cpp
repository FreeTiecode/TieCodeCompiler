#include "program.h"

#include <string>
#include <unordered_set>

namespace tiecodec {

void Program::BuildSymbols(const SemanticModel& semantics, Diagnostics* diagnostics) {
  for (const Unit& unit : units)
    symbols.AddUnit(unit, semantics, diagnostics);
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

bool ValidateSymbolPointers(const Program& program, Diagnostics* diagnostics) {
  std::unordered_set<const ClassDecl*> declarations;
  declarations.reserve(program.symbols.class_count());
  for (const Unit& unit : program.units) {
    for (const ClassDecl& klass : unit.classes)
      declarations.insert(&klass);
  }
  for (const auto& entry : program.symbols.classes()) {
    const ClassInfo& info = entry.second;
    if (info.declaration == nullptr || declarations.count(info.declaration) == 0) {
      if (diagnostics != nullptr) {
        diagnostics->Fatal("内部错误: 类符号指针失效 \"" + info.qualified_name + "\"",
                           "internal.symbol-pointer");
      }
      return false;
    }
    const auto member_is_declared = [&info](const Member* member) {
      for (const Member& declared : info.declaration->members) {
        if (&declared == member)
          return true;
      }
      return false;
    };
    for (const auto& member_entry : info.member_by_name) {
      if (member_entry.second == nullptr || !member_is_declared(member_entry.second)) {
        if (diagnostics != nullptr) {
          diagnostics->Fatal("内部错误: 成员符号指针失效 \"" + info.qualified_name + "." +
                                 member_entry.first + "\"",
                             "internal.symbol-pointer");
        }
        return false;
      }
    }
  }
  return true;
}

}
