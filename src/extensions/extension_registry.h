#ifndef TIECODEC_EXTENSIONS_EXTENSION_REGISTRY_H
#define TIECODEC_EXTENSIONS_EXTENSION_REGISTRY_H

#include <memory>
#include <string>

#include "annotation_registry.h"
#include "requirement_registry.h"

namespace tiecodec {

class ExtensionRegistry {
public:
  bool Register(std::unique_ptr<AnnotationProcessor> processor) {
    if (!annotations_.Register(std::move(processor))) {
      ok_ = false;
      error_ = "重复注册注解处理器";
      return false;
    }
    return true;
  }

  bool Register(std::unique_ptr<PlatformRequirement> requirement) {
    if (!requirements_.Register(std::move(requirement))) {
      ok_ = false;
      error_ = "重复注册平台需求";
      return false;
    }
    return true;
  }

  bool ok() const {
    return ok_;
  }

  const std::string& error() const {
    return error_;
  }

  const AnnotationRegistry& annotations() const {
    return annotations_;
  }

  AnnotationRegistry& annotations() {
    return annotations_;
  }

  const RequirementRegistry& requirements() const {
    return requirements_;
  }

  RequirementRegistry& requirements() {
    return requirements_;
  }

private:
  AnnotationRegistry annotations_;
  RequirementRegistry requirements_;
  bool ok_ = true;
  std::string error_;
};

}

#endif
