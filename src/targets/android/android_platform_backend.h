#ifndef TIECODEC_TARGETS_ANDROID_ANDROID_PLATFORM_BACKEND_H
#define TIECODEC_TARGETS_ANDROID_ANDROID_PLATFORM_BACKEND_H

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "android_java_extension.h"
#include "android_resource_binder.h"
#include "diagnostics.h"
#include "line_map.h"
#include "target.h"
#include "target_config.h"

namespace tiecodec {

class AndroidPlatformBackend : public PlatformBackend {
public:
  AndroidPlatformBackend(const GenerationInput& input,
                         std::shared_ptr<AndroidJavaExtension> extension);

  TargetKey Key() const override;
  bool Generate(const GenerationInput& input, LanguageBackend& language, Diagnostics& diagnostics,
                OutputSink& sink) override;

private:
  bool GenerateInternal(const GenerationInput& input, const GeneratedProgram& program);

  bool WriteFile(const std::string& relative_path, const std::string& content);
  bool WriteProjectFiles(const GenerationInput& input, const GeneratedProgram& program);
  bool GradleMode() const;
  std::string MainPrefix() const;
  std::string SourceDir() const;
  std::string LibsDir() const;
  std::string ReadTemplate(const std::string& relative) const;
  bool WriteGradleProject();
  struct LibraryEntry {
    std::string file;
    bool compile_only = false;
    bool copied = false;
  };
  void PrepareLibraries();
  std::vector<LibraryEntry> library_entries_;
  SourceMapping source_mapping_;
  std::string AppBuildGradle() const;
  std::string LibsJson() const;

  const SymbolTable& symbols_;
  const EffectStore& effects_;
  TargetConfig options_;
  std::shared_ptr<AndroidJavaExtension> extension_;
  AndroidResourceBinder* resources_ = nullptr;
  OutputSink* sink_ = nullptr;
  Diagnostics* diagnostics_ = nullptr;
  const std::set<const Member*>* used_members_ = nullptr;
  const std::vector<const ClassInfo*>* output_classes_ = nullptr;
};

}

#endif
