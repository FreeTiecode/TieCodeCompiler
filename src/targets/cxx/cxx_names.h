#ifndef TIECODEC_TARGETS_CXX_CXX_NAMES_H
#define TIECODEC_TARGETS_CXX_CXX_NAMES_H

#include <map>
#include <string>
#include <vector>

#include "naming.h"
#include "semantic_model.h"
#include "symbols.h"
#include "target_config.h"

namespace tiecodec {

class CxxNames : public NamingScheme {
public:
  CxxNames(const SymbolTable& symbols, const TargetConfig& config, const SemanticModel& semantics);

  std::string ClassName(const ClassInfo& klass) const override;
  std::string MemberName(const Member& member) const override;
  std::string BaseName(const ClassInfo& klass) const override;
  std::string Package(const ClassInfo& klass) const override;
  std::string ImportPath(const ClassInfo& klass) const override;
  std::string GeneratedName(const std::string& hint, int index) const override;
  std::string LocalName(const std::string& name) const override;

  std::string HeaderGuard(const ClassInfo& klass) const;

  bool ReferenceShape() const;

private:
  std::vector<std::string> NamespaceSegments(const ClassInfo& klass) const;
  std::string NamespacePath(const ClassInfo& klass) const;
  std::string Qualify(const ClassInfo& klass) const;
  bool PinyinMode() const;
  bool RandomMode() const;
  std::string RandomName(const std::string& source) const;
  std::string PinyinMemberName(const Member& member) const;

  const SymbolTable& symbols_;
  const TargetConfig& config_;
  const SemanticModel& semantics_;
  mutable std::map<std::string, std::string> random_names_;
  mutable int random_counter_ = 0;
};

std::vector<std::string> SplitCxxNamespace(const std::string& package);

std::string CxxHeaderGuardOf(const std::string& ns_path, const std::string& class_name,
                             bool reference_shape);

std::string CxxOperatorMethodName(const std::string& symbol);

}

#endif
