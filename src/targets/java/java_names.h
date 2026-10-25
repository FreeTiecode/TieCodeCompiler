#ifndef TIECODEC_BACKEND_JAVA_NAMES_H
#define TIECODEC_BACKEND_JAVA_NAMES_H

#include <map>
#include <string>

#include "naming.h"
#include "semantic_model.h"
#include "symbols.h"
#include "target_config.h"

namespace tiecodec {

std::string PinyinLowerCamel(const std::string& identifier);

std::string PinyinCamel(const std::string& identifier);

class JavaNames : public NamingScheme {
public:
  JavaNames(const SymbolTable& symbols, const TargetConfig& config, const SemanticModel& semantics);

  std::string ClassName(const ClassInfo& klass) const override;
  std::string MemberName(const Member& member) const override;
  std::string BaseName(const ClassInfo& klass) const override;
  std::string Package(const ClassInfo& klass) const override;
  std::string ImportPath(const ClassInfo& klass) const override;
  std::string GeneratedName(const std::string& hint, int index) const override;

  std::string AsyncContextName(const std::string& source_class_name,
                               const std::string& member_name) const override;

  std::string AsyncMethodName(const Member& member) const override;

  std::string LocalName(const std::string& name) const override;
  bool LoadStableNames(const std::string& path) override;
  bool SaveNames(const std::string& path) const override;

private:
  std::string ClassSimpleName(const std::string& simple) const;
  std::string MemberSimpleName(const std::string& name) const;
  std::string PackageName(const std::string& package) const;
  std::string RandomName(const std::string& source) const;

  const SymbolTable& symbols_;
  const TargetConfig& config_;
  const SemanticModel& semantics_;
  mutable std::map<std::string, std::string> random_names_;
  mutable std::map<std::string, std::string> stable_names_;
  mutable int random_counter_ = 0;
};

}

#endif
