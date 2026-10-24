#ifndef TIECODEC_CLI_APP_CONFIG_H
#define TIECODEC_CLI_APP_CONFIG_H

#include <string>

#include "backend.h"

namespace tiecodec {

bool LoadAppConfig(const std::string& path, BackendConfig* config, std::string* error);

}

#endif
