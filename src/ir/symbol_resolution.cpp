#include "symbol_resolution.h"

#include <set>

namespace tiecodec {

const Member* FindMemberInChain(const SymbolTable& symbols, const ClassInfo& klass,
                                const std::string& name, const ClassInfo** owner) {
  const ClassInfo* current = &klass;
  const std::size_t max_depth = symbols.class_count() + 1;
  for (std::size_t depth = 0; current != nullptr && current->declaration != nullptr; ++depth) {
    if (depth == max_depth)
      return nullptr;
    const auto member = current->member_by_name.find(name);
    if (member != current->member_by_name.end()) {
      if (owner != nullptr)
        *owner = current;
      return member->second;
    }
    if (current->declaration->base_name.empty())
      break;
    current = symbols.Resolve(current->declaration->base_name, current);
  }
  return nullptr;
}

const Member* FindGlobalClassMember(const SymbolTable& symbols, const ClassInfo& context,
                                    const std::string& name, const ClassInfo** owner) {
  const std::vector<GlobalMemberRef>* entries = symbols.GlobalMembers(name);
  if (entries == nullptr)
    return nullptr;
  for (const GlobalMemberRef& entry : *entries) {
    if (entry.owner == &context)
      continue;
    const Member& member = *entry.member;
    if (member.kind != MemberKind::kMethod && member.kind != MemberKind::kVariable &&
        member.kind != MemberKind::kConstant && member.kind != MemberKind::kPropertyRead) {
      continue;
    }
    if (owner != nullptr)
      *owner = entry.owner;
    return &member;
  }
  return nullptr;
}

const Member* ResolveGlobalMethod(const SymbolTable& symbols, const ClassInfo& context,
                                  const std::string& name, const ClassInfo** owner) {
  if (const Member* member = FindMemberInChain(symbols, context, name, owner))
    return member;
  for (const ClassInfo* candidate : symbols.global_classes()) {
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
