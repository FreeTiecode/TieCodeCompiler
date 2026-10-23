#ifndef TIECODEC_CLI_APP_CONFIG_H
#define TIECODEC_CLI_APP_CONFIG_H

#include <string>

#include "backend.h"

namespace tiecodec {

/// Read an --android.app.config JSON file into the backend configuration.
bool LoadAppConfig(const std::string& path, BackendConfig* config, std::string* error);

}  // namespace tiecodec

#endif  // TIECODEC_CLI_APP_CONFIG_H
