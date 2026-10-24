#ifndef TIECODEC_CLI_RETRACE_H
#define TIECODEC_CLI_RETRACE_H

#include <string>

namespace tiecodec {

struct RetraceRequest {
  std::string mapping_path;
  std::string file;
  std::string line;
};

int RunRetrace(const RetraceRequest& request);

int RunRetraceBatch(const std::string& batch_path);

}

#endif
