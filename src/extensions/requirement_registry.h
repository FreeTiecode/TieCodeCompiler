#ifndef TIECODEC_EXTENSIONS_REQUIREMENT_REGISTRY_H
#define TIECODEC_EXTENSIONS_REQUIREMENT_REGISTRY_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "requirement.h"
#include "target_key.h"

namespace tiecodec {

class RequirementRegistry {
public:
  bool Register(std::unique_ptr<PlatformRequirement> requirement);

  std::vector<const PlatformRequirement*> Matching(const TargetKey& target) const;

  std::size_t size() const {
    return entries_.size();
  }

private:
  struct Entry {
    std::unique_ptr<PlatformRequirement> requirement;
    RequirementMetadata metadata;
  };

  std::vector<Entry> entries_;
};

}

#endif
