#include "requirement_registry.h"

#include <algorithm>

namespace tiecodec {

bool RequirementRegistry::Register(std::unique_ptr<PlatformRequirement> requirement) {
  if (requirement == nullptr)
    return false;
  RequirementMetadata metadata = requirement->Metadata();
  if (metadata.id.empty())
    return false;
  const auto duplicate =
      std::find_if(entries_.begin(), entries_.end(),
                   [&metadata](const Entry& entry) { return entry.metadata.id == metadata.id; });
  if (duplicate != entries_.end())
    return false;
  Entry entry;
  entry.requirement = std::move(requirement);
  entry.metadata = std::move(metadata);
  entries_.push_back(std::move(entry));
  return true;
}

std::vector<const PlatformRequirement*> RequirementRegistry::Matching(
    const TargetKey& target) const {
  std::vector<const PlatformRequirement*> result;
  for (const Entry& entry : entries_) {
    if (entry.metadata.target.Matches(target))
      result.push_back(entry.requirement.get());
  }
  return result;
}

}
