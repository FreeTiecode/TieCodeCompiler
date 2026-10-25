#include "target_registry.h"

#include <algorithm>

namespace tiecodec {

bool TargetRegistry::Register(TargetDescriptor descriptor) {
  const auto duplicate =
      std::find_if(descriptors_.begin(), descriptors_.end(), [&descriptor](const auto& entry) {
        return entry.key.platform == descriptor.key.platform &&
               entry.key.language == descriptor.key.language;
      });
  if (duplicate != descriptors_.end())
    return false;
  descriptors_.push_back(std::move(descriptor));
  return true;
}

const TargetDescriptor* TargetRegistry::Find(const std::string& platform,
                                             const std::string& language) const {
  std::string effective = language;
  if (effective.empty())
    effective = DefaultLanguage(platform);
  for (const TargetDescriptor& descriptor : descriptors_) {
    if (descriptor.key.platform == platform && descriptor.key.language == effective)
      return &descriptor;
  }
  return nullptr;
}

std::vector<std::string> TargetRegistry::platforms() const {
  std::vector<std::string> result;
  for (const TargetDescriptor& descriptor : descriptors_) {
    if (std::find(result.begin(), result.end(), descriptor.key.platform) == result.end())
      result.push_back(descriptor.key.platform);
  }
  return result;
}

std::vector<std::string> TargetRegistry::languages(const std::string& platform) const {
  std::vector<std::string> result;
  for (const TargetDescriptor& descriptor : descriptors_) {
    if (descriptor.key.platform == platform)
      result.push_back(descriptor.key.language);
  }
  return result;
}

std::string TargetRegistry::DefaultLanguage(const std::string& platform) const {
  for (const TargetDescriptor& descriptor : descriptors_) {
    if (descriptor.key.platform == platform)
      return descriptor.key.language;
  }
  return {};
}

}
