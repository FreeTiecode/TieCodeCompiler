#ifndef TIECODEC_CLI_APP_CONFIG_H
#define TIECODEC_CLI_APP_CONFIG_H

#include <string>

#include "target_config.h"

namespace tiecodec {

bool LoadAppConfig(const std::string& path, TargetConfig* config, std::string* error);

}

#endif
