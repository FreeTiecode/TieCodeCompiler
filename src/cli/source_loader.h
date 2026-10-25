#ifndef TIECODEC_CLI_SOURCE_LOADER_H
#define TIECODEC_CLI_SOURCE_LOADER_H

#include <string>

namespace tiecodec {

struct CliCommand;
struct Diagnostics;
struct Program;

enum class SourceLoadStatus {
  kOk,
  kNoSources,
  kScanError,
};

SourceLoadStatus LoadProgramSources(const CliCommand& command, Program* program,
                                    Diagnostics* diagnostics, std::string* error);

}

#endif
