#ifndef TIECODEC_BACKEND_ANDROID_BACKEND_H
#define TIECODEC_BACKEND_ANDROID_BACKEND_H

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "android_environment_contract.h"
#include "android_resource_binder.h"
#include "backend.h"
#include "diagnostics.h"
#include "java_emitter.h"
#include "java_names.h"
#include "line_map.h"
#include "naming.h"
#include "type_mapper.h"

namespace tiecodec {

class AndroidBackend : public Backend {
public:
  explicit AndroidBackend(const SymbolTable& symbols);

  std::string Name() const override {
    return "android";
  }
  std::string DisplayName() const override {
    return "Android";
  }
  const EnvironmentContract& Contract() const override {
    return contract_;
  }
  bool Generate(const ClassInfo* entry, const BackendConfig& config,
                Diagnostics& diagnostics) override;

private:
  bool GenerateInternal(const ClassInfo* entry);

  bool WriteFile(const std::string& relative_path, const std::string& content);
  bool WriteClassFiles(const ClassInfo* klass, ClassOutput* output,
                       std::set<const ClassInfo*>* emitted_classes);
  bool WriteProjectFiles(const ClassInfo& entry);
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
  void RecordLineMap(const ClassInfo& klass, const ClassOutput& output);
  std::uint64_t SourceFileIndex(const std::string& file);
  void CollectMappingNames();
  std::vector<LibraryEntry> library_entries_;
  SourceMapping source_mapping_;
  std::map<std::string, std::uint64_t> source_file_index_;
  std::string AppBuildGradle() const;
  std::string LibsJson() const;

  const SymbolTable& symbols_;
  BackendConfig options_;
  AndroidEnvironmentContract contract_;
  std::unique_ptr<NamingScheme> names_;
  std::unique_ptr<TypeMapper> types_;
  std::unique_ptr<JavaEmitter> emitter_;
  std::unique_ptr<AndroidResourceBinder> resources_;
  std::unique_ptr<OutputSink> sink_;
  Diagnostics* diagnostics_ = nullptr;
  const std::set<const Member*>* used_members_ = nullptr;
  const std::vector<const ClassInfo*>* output_classes_ = nullptr;
};

}

#endif
