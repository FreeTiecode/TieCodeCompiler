#ifndef TIECODEC_IR_PLAN_HINTS_H
#define TIECODEC_IR_PLAN_HINTS_H

#include <algorithm>
#include <vector>

#include "symbols.h"

namespace tiecodec {

struct PlanHints {
  std::vector<const ClassInfo*> forced_classes;
  std::vector<GlobalMemberRef> forced_members;
  std::vector<const ClassInfo*> referenced_classes;
  std::vector<GlobalMemberRef> referenced_members;
  std::vector<GlobalMemberRef> virtual_members;

  void ForceClass(const ClassInfo* klass) {
    if (klass != nullptr &&
        std::find(forced_classes.begin(), forced_classes.end(), klass) == forced_classes.end())
      forced_classes.push_back(klass);
  }

  void ForceMember(const ClassInfo* owner, const Member* member) {
    AddUnique(&forced_members, owner, member);
  }

  void ReferenceClass(const ClassInfo* klass) {
    if (klass != nullptr && std::find(referenced_classes.begin(), referenced_classes.end(),
                                      klass) == referenced_classes.end())
      referenced_classes.push_back(klass);
  }

  void ReferenceMember(const ClassInfo* owner, const Member* member) {
    AddUnique(&referenced_members, owner, member);
  }

  void AddVirtual(const ClassInfo* owner, const Member* member) {
    AddUnique(&virtual_members, owner, member);
  }

private:
  static void AddUnique(std::vector<GlobalMemberRef>* target, const ClassInfo* owner,
                        const Member* member) {
    if (owner == nullptr || member == nullptr)
      return;
    const auto found =
        std::find_if(target->begin(), target->end(), [owner, member](const GlobalMemberRef& entry) {
          return entry.owner == owner && entry.member == member;
        });
    if (found == target->end())
      target->push_back(GlobalMemberRef{owner, member});
  }
};

}

#endif
