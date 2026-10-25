#include "console_reporter.h"

#include <iostream>

namespace tiecodec {

int ConsoleReporter::ReportCheck(const Diagnostics& diagnostics, std::size_t unit_count) {
  diagnostics.Flush(console_);
  const int failure_count = diagnostics.error_count() + diagnostics.fatal_count();
  if (failure_count > 0) {
    std::cerr << "解析失败，存在 " << failure_count << " 个错误\n";
    return 1;
  }
  std::cout << "解析通过，共 " << unit_count << " 个源文件\n";
  return 0;
}

int ConsoleReporter::ReportCompile(const Diagnostics& diagnostics, bool ok) {
  diagnostics.Flush(console_);
  if (ok) {
    if (diagnostics.warning_count() > 0) {
      std::cout << "编译成功，但存在 " << diagnostics.warning_count() << " 个警告\n";
    } else {
      std::cout << "编译完成\n";
    }
    return 0;
  }
  if (diagnostics.fatal_count() > 0) {
    std::cout << "编译失败，存在致命错误\n";
  } else {
    std::cout << "编译失败，存在 " << diagnostics.error_count() << " 个错误\n";
  }
  return 1;
}

}
