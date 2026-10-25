#ifndef TIECODEC_TARGETS_JVM_JVM_PLATFORM_BACKEND_H
#define TIECODEC_TARGETS_JVM_JVM_PLATFORM_BACKEND_H

#include <map>
#include <string>

#include "diagnostics.h"
#include "line_map.h"
#include "target.h"

namespace tiecodec {

class JvmPlatformBackend : public PlatformBackend {
public:
  explicit JvmPlatformBackend(const GenerationInput& input);

  TargetKey Key() const override;
  bool Generate(const GenerationInput& input, LanguageBackend& language, Diagnostics& diagnostics,
                OutputSink& sink) override;

private:
  bool WriteFile(OutputSink& sink, const std::string& relative_path,
                 const std::string& content) const;
  std::string LauncherSource(const GenerationInput& input, const GeneratedProgram& program) const;
  std::string Metadata(const GenerationInput& input, const GeneratedProgram& program) const;
  void WriteLineMap(const GenerationInput& input, const GeneratedProgram& program) const;

  TargetConfig options_;
  Diagnostics* diagnostics_ = nullptr;
};

}

#endif
