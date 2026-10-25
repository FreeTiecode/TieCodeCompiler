#include "semantic_model.h"

namespace tiecodec {

UnitFacts& SemanticModel::UnitFactsFor(const Unit* unit) {
  return units_[unit];
}

ClassFacts& SemanticModel::ClassFactsFor(const ClassDecl* klass) {
  return classes_[klass];
}

MemberFacts& SemanticModel::MemberFactsFor(const Member* member) {
  return members_[member];
}

const UnitFacts* SemanticModel::FindUnit(const Unit* unit) const {
  const auto it = units_.find(unit);
  return it == units_.end() ? nullptr : &it->second;
}

const ClassFacts* SemanticModel::FindClass(const ClassDecl* klass) const {
  const auto it = classes_.find(klass);
  return it == classes_.end() ? nullptr : &it->second;
}

const MemberFacts* SemanticModel::FindMember(const Member* member) const {
  const auto it = members_.find(member);
  return it == members_.end() ? nullptr : &it->second;
}

const std::string* SemanticModel::ExternalType(const ClassDecl* klass,
                                               const std::string& language) const {
  const ClassFacts* facts = FindClass(klass);
  if (facts == nullptr)
    return nullptr;
  const auto it = facts->external_types.find(language);
  return it == facts->external_types.end() ? nullptr : &it->second;
}

}
