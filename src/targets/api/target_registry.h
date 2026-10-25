#ifndef TIECODEC_TARGETS_API_TARGET_REGISTRY_H
#define TIECODEC_TARGETS_API_TARGET_REGISTRY_H

#include <string>
#include <vector>

#include "target.h"
#include "target_key.h"

namespace tiecodec {

class TargetRegistry {
public:
  bool Register(TargetDescriptor descriptor);

  const TargetDescriptor* Find(const std::string& platform,
                               const std::string& language = std::string()) const;

  std::vector<std::string> platforms() const;
  std::vector<std::string> languages(const std::string& platform) const;
  std::string DefaultLanguage(const std::string& platform) const;

private:
  std::vector<TargetDescriptor> descriptors_;
};

}

#endif
