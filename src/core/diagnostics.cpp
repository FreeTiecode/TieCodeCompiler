#include "diagnostics.h"

#include <cstdio>

#include "position.h"

namespace tiecodec {
namespace {

const char* SeverityPrefix(Diagnostic::Severity severity) {
  switch (severity) {
    case Diagnostic::Severity::kInfo:
      return "[INFO] ";
    case Diagnostic::Severity::kWarning:
      return "[WARNING] ";
    case Diagnostic::Severity::kError:
      return "[ERROR] ";
    case Diagnostic::Severity::kFatal:
      return "[ERROR] ";
  }
  return "[INFO] ";
}

const char* SeverityLabel(Diagnostic::Severity severity) {
  switch (severity) {
    case Diagnostic::Severity::kInfo:
      return "提示";
    case Diagnostic::Severity::kWarning:
      return "警告";
    case Diagnostic::Severity::kError:
      return "错误";
    case Diagnostic::Severity::kFatal:
      return "错误";
  }
  return "提示";
}

}

void ConsoleDiagnosticReporter::Report(const Diagnostic& diagnostic) {
  std::string line = SeverityPrefix(diagnostic.severity);
  if (diagnostic.has_location()) {
    line += diagnostic.file;
    line.push_back(':');
    line += SeverityLabel(diagnostic.severity);
    line += '(' + std::to_string(diagnostic.line + 1) + ',' +
            std::to_string(diagnostic.column + 1) + "): ";
    line += diagnostic.message;
  } else {
    line += SeverityLabel(diagnostic.severity);
    line += "：";
    line += diagnostic.message;
  }
  line.push_back('\n');
  const bool is_error = diagnostic.severity == Diagnostic::Severity::kError ||
                        diagnostic.severity == Diagnostic::Severity::kFatal;
  std::FILE* stream = is_error ? stderr : stdout;
  std::fwrite(line.data(), 1, line.size(), stream);
  std::fflush(stream);
}

void Diagnostics::Add(Diagnostic diagnostic) {
  switch (diagnostic.severity) {
    case Diagnostic::Severity::kInfo:
      break;
    case Diagnostic::Severity::kWarning:
      ++warning_count_;
      break;
    case Diagnostic::Severity::kError:
      ++error_count_;
      break;
    case Diagnostic::Severity::kFatal:
      ++fatal_count_;
      break;
  }
  diagnostics_.push_back(std::move(diagnostic));
}

void Diagnostics::Error(const std::string& file, const SourcePosition& position,
                        const std::string& message, const std::string& key) {
  Diagnostic diagnostic;
  diagnostic.severity = Diagnostic::Severity::kError;
  diagnostic.file = file;
  diagnostic.line = position.line;
  diagnostic.column = position.column;
  diagnostic.key = key;
  diagnostic.message = message;
  Add(std::move(diagnostic));
}

void Diagnostics::ErrorGlobal(const std::string& message, const std::string& key) {
  Diagnostic diagnostic;
  diagnostic.severity = Diagnostic::Severity::kError;
  diagnostic.key = key;
  diagnostic.message = message;
  Add(std::move(diagnostic));
}

void Diagnostics::Warning(const std::string& file, const SourcePosition& position,
                          const std::string& message, const std::string& key) {
  Diagnostic diagnostic;
  diagnostic.severity = Diagnostic::Severity::kWarning;
  diagnostic.file = file;
  diagnostic.line = position.line;
  diagnostic.column = position.column;
  diagnostic.key = key;
  diagnostic.message = message;
  Add(std::move(diagnostic));
}

void Diagnostics::Fatal(const std::string& message, const std::string& key) {
  Diagnostic diagnostic;
  diagnostic.severity = Diagnostic::Severity::kFatal;
  diagnostic.key = key;
  diagnostic.message = message;
  Add(std::move(diagnostic));
}

void Diagnostics::Flush(DiagnosticReporter& reporter) const {
  for (const Diagnostic& diagnostic : diagnostics_)
    reporter.Report(diagnostic);
}

}
