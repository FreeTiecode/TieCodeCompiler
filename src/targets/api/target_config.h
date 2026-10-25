#ifndef TIECODEC_TARGETS_API_TARGET_CONFIG_H
#define TIECODEC_TARGETS_API_TARGET_CONFIG_H

#include <map>
#include <string>
#include <vector>

#include "constants.h"
#include "options.h"
#include "text_util.h"

namespace tiecodec {

struct TargetConfig {
  std::string output_dir;
  std::string package_name = kDefaultPackageName;
  std::string namespace_name = kDefaultNamespaceName;
  std::string app_name = kDefaultAppName;
  std::string app_icon;
  std::string version_name = kDefaultVersionName;
  std::string friendly_name = kFriendlyNameOriginal;
  std::string profile = "standard";
  int min_sdk = kDefaultMinSdk;
  int target_sdk = kDefaultTargetSdk;
  int version_code = kDefaultVersionCode;
  std::map<std::string, std::string> options;

  bool Flag(const std::string& key) const {
    const auto it = options.find(key);
    return it != options.end() && it->second == "1";
  }

  std::string Option(const std::string& key, const std::string& fallback = std::string()) const {
    const auto it = options.find(key);
    return it == options.end() ? fallback : it->second;
  }

  std::vector<std::string> ListOption(const std::string& key, char delimiter = ';') const {
    const auto it = options.find(key);
    return it == options.end() ? std::vector<std::string>() : SplitDelimited(it->second, delimiter);
  }
};

}

#endif
