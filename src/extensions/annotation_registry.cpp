#include "annotation_registry.h"

#include <algorithm>
#include <limits>

namespace tiecodec {

bool AnnotationRegistry::Register(std::unique_ptr<AnnotationProcessor> processor) {
  if (processor == nullptr)
    return false;
  AnnotationMetadata metadata = processor->Metadata();
  if (metadata.name.empty())
    return false;
  for (const Entry& entry : entries_) {
    if (entry.metadata.name == metadata.name &&
        entry.metadata.target.platform == metadata.target.platform &&
        entry.metadata.target.language == metadata.target.language) {
      return false;
    }
  }
  Entry entry;
  entry.processor = std::move(processor);
  entry.metadata = std::move(metadata);
  entry.order = entries_.size();
  entries_.push_back(std::move(entry));
  return true;
}

const AnnotationProcessor* AnnotationRegistry::Find(std::string_view name,
                                                    const TargetKey& target) const {
  const AnnotationProcessor* best = nullptr;
  int best_specificity = -1;
  int best_priority = std::numeric_limits<int>::min();
  std::size_t best_order = std::numeric_limits<std::size_t>::max();
  for (const Entry& entry : entries_) {
    if (entry.metadata.name != name)
      continue;
    if (!entry.metadata.target.Matches(target))
      continue;
    const int specificity = entry.metadata.target.Specificity(target);
    if (specificity > best_specificity ||
        (specificity == best_specificity && entry.metadata.priority > best_priority) ||
        (specificity == best_specificity && entry.metadata.priority == best_priority &&
         entry.order < best_order)) {
      best = entry.processor.get();
      best_specificity = specificity;
      best_priority = entry.metadata.priority;
      best_order = entry.order;
    }
  }
  return best;
}

bool AnnotationRegistry::HasName(std::string_view name) const {
  return std::any_of(entries_.begin(), entries_.end(),
                     [name](const Entry& entry) { return entry.metadata.name == name; });
}

std::vector<std::string> AnnotationRegistry::names() const {
  std::vector<std::string> result;
  for (const Entry& entry : entries_) {
    if (std::find(result.begin(), result.end(), entry.metadata.name) == result.end())
      result.push_back(entry.metadata.name);
  }
  std::sort(result.begin(), result.end());
  return result;
}

}
