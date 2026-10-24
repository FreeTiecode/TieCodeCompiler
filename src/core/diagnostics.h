#ifndef TIECODEC_CORE_DIAGNOSTICS_H
#define TIECODEC_CORE_DIAGNOSTICS_H

#include <cstddef>
#include <string>
#include <vector>

#include "position.h"

namespace tiecodec {

struct Diagnostic {
  enum class Severity { kInfo, kWarning, kError, kFatal };

  Severity severity = Severity::kError;
  std::string file;
  std::size_t line = 0;
  std::size_t column = 0;
  std::string key;
  std::string message;

  bool has_location() const {
    return !file.empty();
  }
};

class DiagnosticReporter {
public:
  virtual ~DiagnosticReporter() = default;
  virtual void Report(const Diagnostic& diagnostic) = 0;
};

class ConsoleDiagnosticReporter : public DiagnosticReporter {
public:
  void Report(const Diagnostic& diagnostic) override;
};

class Diagnostics {
public:
  void Error(const std::string& file, const SourcePosition& position, const std::string& message,
             const std::string& key = "parse");
  void ErrorGlobal(const std::string& message, const std::string& key = "parse");
  void Warning(const std::string& file, const SourcePosition& position, const std::string& message,
               const std::string& key = "parse");
  void Fatal(const std::string& message, const std::string& key = "parse");

  int error_count() const {
    return error_count_;
  }
  int warning_count() const {
    return warning_count_;
  }
  int fatal_count() const {
    return fatal_count_;
  }
  bool has_errors() const {
    return error_count_ > 0 || fatal_count_ > 0;
  }

  const std::vector<Diagnostic>& all() const {
    return diagnostics_;
  }

  void Flush(DiagnosticReporter& reporter) const;

private:
  void Add(Diagnostic diagnostic);
  std::vector<Diagnostic> diagnostics_;
  int error_count_ = 0;
  int warning_count_ = 0;
  int fatal_count_ = 0;
};

}

#endif
