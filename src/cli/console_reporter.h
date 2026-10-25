#ifndef TIECODEC_CLI_CONSOLE_REPORTER_H
#define TIECODEC_CLI_CONSOLE_REPORTER_H

#include <cstddef>

#include "diagnostics.h"

namespace tiecodec {

class ConsoleReporter {
public:
  int ReportCheck(const Diagnostics& diagnostics, std::size_t unit_count);

  int ReportCompile(const Diagnostics& diagnostics, bool ok);

private:
  ConsoleDiagnosticReporter console_;
};

}

#endif
