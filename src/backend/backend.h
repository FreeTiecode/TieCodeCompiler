#ifndef TIECODEC_BACKEND_BACKEND_H
#define TIECODEC_BACKEND_BACKEND_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "diagnostics.h"
#include "environment_contract.h"
#include "output_sink.h"
#include "symbols.h"

namespace tiecodec {

struct BackendConfig {
  std::string output_dir;
  std::string package_name = "js.zw";
  std::string namespace_name = "结绳.中文";
  std::string app_name = "MyApp";
  std::string app_icon;
  std::string version_name = "1.0";
  std::string friendly_name = "original";
  int min_sdk = 21;
  int target_sdk = 28;
  int version_code = 1;
  std::map<std::string, std::string> options;
};

class Backend {
public:
  virtual ~Backend() = default;

  virtual std::string Name() const = 0;

  virtual std::string DisplayName() const {
    return Name();
  }

  virtual const EnvironmentContract& Contract() const = 0;

  virtual bool Generate(const ClassInfo* entry, const BackendConfig& config,
                        Diagnostics& diagnostics) = 0;
};

}  // namespace tiecodec

#endif
