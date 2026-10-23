#ifndef TIECODEC_BACKEND_ANDROID_RESOURCE_BINDER_H
#define TIECODEC_BACKEND_ANDROID_RESOURCE_BINDER_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include "backend.h"
#include "java_names.h"
#include "java_type_mapper.h"
#include "resource_binder.h"
#include "symbols.h"

namespace tiecodec {

class OutputSink;

/// Android implementation of ResourceBinder: turns resource-typed constants into
/// R references and the matching res/ files, internationalized strings, themes,
/// extra resources and manifest fragments.
class AndroidResourceBinder : public ResourceBinder {
public:
  AndroidResourceBinder(const SymbolTable& symbols, const BackendConfig& config);

  bool IsResourceField(const ClassInfo& owner, const Member& member) const override;
  std::string FieldType(const ClassInfo& owner, const Member& member) const override;
  std::string BindField(const ClassInfo& owner, const Member& member) override;
  std::string ResourceImport(const ClassInfo& owner) const override;

  /// Platform annotations of an emitted class (resources, libraries, manifest).
  void set_output_prefix(std::string prefix) {
    output_prefix_ = std::move(prefix);
  }
  void set_source_prefix(std::string prefix) {
    source_prefix_ = std::move(prefix);
  }

  void ProcessClassAnnotations(const ClassInfo& klass);
  /// Manifest fragments attached to an emitted used member.
  void ProcessMemberManifest(const ClassInfo& klass, const Member& member);

  /// Files represented by the collected resources (strings/styles/copies).
  void Write(OutputSink& sink) const;

  bool HasResources() const {
    return !resources_empty_ && (has_res_ || has_strings_ || has_styles_);
  }
  const std::vector<std::string>& application_children() const {
    return application_children_;
  }
  const std::string& global_theme() const {
    return global_theme_;
  }

private:
  struct FileCopy {
    std::string source;
    std::string target;
  };
  struct StringEntry {
    std::string name;
    std::string value;
    std::string sort_key;
  };
  struct StyleItem {
    std::string name;
    std::string value;
  };
  struct StyleEntry {
    std::string name;
    std::string parent;
    std::vector<StyleItem> items;
  };

  std::string RuleOf(const ClassInfo& klass) const;
  std::string ConstantTypeOf(const ClassInfo& klass) const;
  std::string LiteralValue(const Member& member) const;
  std::string OutputNameOf(const Member& member) const;
  bool RegisterFileResource(const ClassInfo& owner, const Member& member, const std::string& rule,
                            const std::string& value);
  bool RegisterStringResource(const Member& member, const std::string& value);
  bool RegisterThemeResource(const ClassInfo& owner, const Member& member,
                             const std::string& value);
  void RegisterOwnedName(const std::string& group, const std::string& identifier,
                         const std::string& value_key, std::string* name);
  std::string ResolvePath(const ClassInfo& owner, const std::string& path) const;
  void CopyDirectory(const std::string& directory, const std::string& target_prefix);
  void CopyResourceDirectory(const std::string& directory);
  void CopyFileTo(const std::string& source, const std::string& target);
  static std::vector<std::pair<std::string, std::string>> ParseTable(const std::string& text);

  const SymbolTable& symbols_;
  const BackendConfig& config_;
  JavaNames names_;
  JavaTypeMapper types_;

  std::map<std::string, std::set<std::string>> used_names_;
  std::map<std::string, std::map<std::string, std::string>> value_names_;
  std::vector<FileCopy> copies_;
  std::set<std::string> directories_;
  std::map<std::string, std::vector<StringEntry>> strings_;
  std::vector<StyleEntry> styles_;
  std::vector<std::string> application_children_;
  std::string global_theme_;
  std::string pending_field_;
  std::string output_prefix_;
  std::string source_prefix_ = "src/";
  bool resources_empty_ = true;
  bool has_res_ = false;
  bool has_strings_ = false;
  bool has_styles_ = false;
};

}  // namespace tiecodec

#endif  // TIECODEC_BACKEND_ANDROID_RESOURCE_BINDER_H
