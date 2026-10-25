#ifndef TIECODEC_EXTENSIONS_ANNOTATION_REGISTRY_H
#define TIECODEC_EXTENSIONS_ANNOTATION_REGISTRY_H

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "annotation.h"
#include "target_key.h"

namespace tiecodec {

class AnnotationRegistry {
public:
  bool Register(std::unique_ptr<AnnotationProcessor> processor);

  const AnnotationProcessor* Find(std::string_view name, const TargetKey& target) const;

  bool HasName(std::string_view name) const;

  std::vector<std::string> names() const;

  std::size_t size() const {
    return entries_.size();
  }

private:
  struct Entry {
    std::unique_ptr<AnnotationProcessor> processor;
    AnnotationMetadata metadata;
    std::size_t order = 0;
  };

  std::vector<Entry> entries_;
};

}

#endif
