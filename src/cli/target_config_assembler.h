#ifndef TIECODEC_CLI_TARGET_CONFIG_ASSEMBLER_H
#define TIECODEC_CLI_TARGET_CONFIG_ASSEMBLER_H

#include <string>

namespace tiecodec {

struct CliCommand;
struct TargetConfig;

bool BuildTargetConfig(const CliCommand& command, TargetConfig* config, std::string* error);

}

#endif
