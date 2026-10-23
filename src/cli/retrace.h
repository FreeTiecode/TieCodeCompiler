#ifndef TIECODEC_CLI_RETRACE_H
#define TIECODEC_CLI_RETRACE_H

#include <string>

namespace tiecodec {

struct RetraceRequest {
  std::string mapping_path;
  std::string file;
  std::string line;
};

/// Single file/line retrace; prints "<source>:<line>" or ":0".
int RunRetrace(const RetraceRequest& request);

/// Batch retrace from a JSON file; prints the reference JSON format.
int RunRetraceBatch(const std::string& batch_path);

}  // namespace tiecodec

#endif  // TIECODEC_CLI_RETRACE_H
