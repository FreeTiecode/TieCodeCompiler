#ifndef TIECODEC_EXTENSIONS_TARGET_KEY_H
#define TIECODEC_EXTENSIONS_TARGET_KEY_H

#include <string>

namespace tiecodec {

struct TargetKey {
  std::string platform;
  std::string language;
};

struct TargetFilter {
  std::string platform;
  std::string language;

  bool Matches(const TargetKey& key) const {
    return (language.empty() || language == key.language) &&
           (platform.empty() || platform == key.platform);
  }

  int Specificity(const TargetKey& key) const {
    int score = 0;
    if (!language.empty() && language == key.language)
      score += 2;
    if (!platform.empty() && platform == key.platform)
      score += 1;
    return score;
  }
};

inline TargetFilter AnyTarget() {
  return {};
}

inline TargetFilter LanguageTarget(std::string language) {
  TargetFilter filter;
  filter.language = std::move(language);
  return filter;
}

inline TargetFilter PlatformTarget(std::string platform) {
  TargetFilter filter;
  filter.platform = std::move(platform);
  return filter;
}

}

#endif
