#include "symbol_resolution.h"

#include <set>

namespace tiecodec {

const Member* FindMemberInChain(const SymbolTable& symbols, const ClassInfo& klass,
                                const std::string& name, const ClassInfo** owner) {
  const ClassInfo* current = &klass;
  std::set<const ClassInfo*> visited;
  while (current != nullptr && current->declaration != nullptr && visited.insert(current).second) {
    for (const Member& member : current->declaration->members) {
      if (member.name != name)
        continue;
      if (owner != nullptr)
        *owner = current;
      return &member;
    }
    if (current->declaration->base_name.empty())
      break;
    current = symbols.Resolve(current->declaration->base_name, current);
  }
  return nullptr;
}

const Member* FindGlobalClassMember(const SymbolTable& symbols, const ClassInfo& context,
                                    const std::string& name, const ClassInfo** owner) {
  for (const std::string& qualified_name : symbols.class_names()) {
    const ClassInfo* candidate = symbols.Find(qualified_name);
    if (candidate == nullptr || candidate == &context || !candidate->is_global_class)
      continue;
    if (candidate->declaration == nullptr)
      continue;
    for (const Member& member : candidate->declaration->members) {
      if (member.name != name)
        continue;
      if (member.kind != MemberKind::kMethod && member.kind != MemberKind::kVariable &&
          member.kind != MemberKind::kConstant && member.kind != MemberKind::kPropertyRead) {
        continue;
      }
      if (owner != nullptr)
        *owner = candidate;
      return &member;
    }
  }
  return nullptr;
}

const Member* ResolveGlobalMethod(const SymbolTable& symbols, const ClassInfo& context,
                                  const std::string& name, const ClassInfo** owner) {
  if (const Member* member = FindMemberInChain(symbols, context, name, owner))
    return member;
  for (const std::string& qualified_name : symbols.class_names()) {
    const ClassInfo* candidate = symbols.Find(qualified_name);
    if (candidate == nullptr || !candidate->is_global_class)
      continue;
    if (const Member* member = FindMemberInChain(symbols, *candidate, name, owner))
      return member;
  }
  return nullptr;
}

const ClassInfo* BaseClassOf(const SymbolTable& symbols, const ClassInfo& klass) {
  if (klass.declaration == nullptr || klass.declaration->base_name.empty())
    return nullptr;
  return symbols.Resolve(klass.declaration->base_name, &klass);
}

bool DerivesFrom(const SymbolTable& symbols, const ClassInfo* derived, const ClassInfo* base) {
  if (derived == nullptr || base == nullptr)
    return false;
  const ClassInfo* cursor = derived;
  std::set<const ClassInfo*> visited;
  while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
    if (cursor == base)
      return true;
    cursor = BaseClassOf(symbols, *cursor);
  }
  return false;
}

}
