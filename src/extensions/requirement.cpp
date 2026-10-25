#include "requirement.h"

#include <string>

#include "diagnostics.h"
#include "semantic_model.h"
#include "symbol_resolution.h"
#include "symbols.h"

namespace tiecodec {
namespace {

std::string SimpleName(const std::string& qualified_name) {
  const std::size_t dot = qualified_name.rfind('.');
  return dot == std::string::npos ? qualified_name : qualified_name.substr(dot + 1);
}

const ClassInfo* FindType(const SymbolTable& symbols, const std::string& name) {
  for (const std::string& qualified_name : symbols.class_names()) {
    if (SimpleName(qualified_name) != name)
      continue;
    return symbols.Find(qualified_name);
  }
  return nullptr;
}

bool HasDirectMember(const ClassInfo& klass, const std::string& name) {
  if (klass.declaration == nullptr)
    return false;
  for (const Member& member : klass.declaration->members) {
    if (member.name == name)
      return true;
  }
  return false;
}

}

DeclarativeRequirement::DeclarativeRequirement(RequirementMetadata metadata)
    : metadata_(std::move(metadata)) {}

RequirementMetadata DeclarativeRequirement::Metadata() const {
  return metadata_;
}

void PlatformRequirement::Validate(const SymbolTable& symbols, Diagnostics& diagnostics) const {
  const RequirementMetadata metadata = Metadata();
  if (metadata.kind == RequirementKind::kType) {
    if (FindType(symbols, metadata.type_name) != nullptr)
      return;
    diagnostics.Fatal(std::string("当前编译环境中缺少编译器必要类型定义类 \"") +
                          metadata.type_name + "\"，此为致命错误",
                      "enter.basic-type.missed");
    return;
  }
  const ClassInfo* owner = FindType(symbols, metadata.owner_class);
  if (owner == nullptr || owner->declaration == nullptr)
    return;
  if (HasDirectMember(*owner, metadata.member_name))
    return;
  diagnostics.Fatal(std::string("类\"") + metadata.owner_class + "\"中缺少成员\"" +
                        metadata.member_name + "\"，编译环境缺失，此为致命错误",
                    "enter.member.missed");
}

void PlatformRequirement::Apply(const SymbolTable& symbols, const SemanticModel& semantics,
                                PlanHints& plan) const {
  const RequirementMetadata metadata = Metadata();
  if (metadata.kind != RequirementKind::kMember)
    return;
  const ClassInfo* owner_class = FindType(symbols, metadata.owner_class);
  if (owner_class == nullptr)
    return;
  const ClassInfo* owner = nullptr;
  const Member* member = FindMemberInChain(symbols, *owner_class, metadata.member_name, &owner);
  if (member == nullptr)
    return;
  if (metadata.seed_use)
    plan.ForceMember(owner, member);
  if (metadata.virtual_hook) {
    const MemberFacts* facts = semantics.FindMember(member);
    if (facts != nullptr && facts->is_virtual)
      plan.AddVirtual(owner, member);
  }
}

}
