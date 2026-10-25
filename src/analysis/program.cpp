#include "program.h"

#include <set>
#include <string>

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
  std::set<const ClassDecl*> declarations;
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
    std::set<const Member*> members;
    for (const Member& member : info.declaration->members)
      members.insert(&member);
    for (const auto& member_entry : info.member_by_name) {
      if (members.count(member_entry.second) == 0) {
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
