#ifndef TIECODEC_TARGETS_CXX_CXX_PLATFORM_BACKEND_H
#define TIECODEC_TARGETS_CXX_CXX_PLATFORM_BACKEND_H

#include <set>
#include <string>
#include <vector>

#include "diagnostics.h"
#include "target.h"
#include "target_config.h"

namespace tiecodec {

class CxxPlatformBackend : public PlatformBackend {
public:
  explicit CxxPlatformBackend(const GenerationInput& input);

  TargetKey Key() const override;
  bool Generate(const GenerationInput& input, LanguageBackend& language, Diagnostics& diagnostics,
                OutputSink& sink) override;

private:
  bool WriteFile(OutputSink& sink, const std::string& relative_path,
                 const std::string& content) const;
  std::string LauncherSource(const GenerationInput& input, const GeneratedProgram& program,
                             bool reference_shape) const;
  std::string CMakeScript(const std::vector<std::string>& headers,
                          const std::vector<std::string>& sources,
                          const std::vector<std::string>& libraries) const;
  std::string GccScript(const std::vector<std::string>& sources,
                        const std::vector<std::string>& libraries) const;
  std::string MsvcScript(const std::vector<std::string>& sources,
                         const std::vector<std::string>& libraries) const;
  std::string Metadata(const GenerationInput& input, const GeneratedProgram& program,
                       bool reference_shape) const;
  void CollectExternalSources(const GenerationInput& input, bool reference_shape,
                              const std::vector<const ClassInfo*>& output_classes,
                              std::set<std::string>* headers, std::set<std::string>* sources,
                              std::set<std::string>* libraries, OutputSink* sink) const;
  std::string EntryQualified(const GeneratedProgram& program, bool reference_shape) const;

  const TargetConfig& config_;
  Diagnostics* diagnostics_ = nullptr;
};

}

#endif
