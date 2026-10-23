#ifndef TIECODEC_BACKEND_JAVA_NAMES_H
#define TIECODEC_BACKEND_JAVA_NAMES_H

#include <map>
#include <string>

#include "backend.h"
#include "naming.h"
#include "symbols.h"

namespace tiecodec {

/// Lower camel case pinyin of an identifier (exposed for embedded code
/// identifier transliteration).
std::string PinyinLowerCamel(const std::string& identifier);

/// Pascal case pinyin of an identifier.
std::string PinyinCamel(const std::string& identifier);

class JavaNames : public NamingScheme {
public:
  JavaNames(const SymbolTable& symbols, const BackendConfig& config);

  std::string ClassName(const ClassInfo& klass) const override;
  std::string MemberName(const Member& member) const override;
  std::string BaseName(const ClassInfo& klass) const override;
  std::string Package(const ClassInfo& klass) const override;
  std::string ImportPath(const ClassInfo& klass) const override;
  std::string GeneratedName(const std::string& hint, int index) const override;

  /// Output package of the compiler generated R class.
  std::string RNamespace() const override;
  std::string LocalName(const std::string& name) const override;
  bool LoadStableNames(const std::string& path) override;
  bool SaveNames(const std::string& path) const override;

private:
  std::string ClassSimpleName(const std::string& simple) const;
  std::string MemberSimpleName(const std::string& name) const;
  std::string PackageName(const std::string& package) const;
  std::string RandomName(const std::string& source) const;

  const SymbolTable& symbols_;
  const BackendConfig& config_;
  mutable std::map<std::string, std::string> random_names_;
  mutable std::map<std::string, std::string> stable_names_;
  mutable int random_counter_ = 0;
};

}  // namespace tiecodec

#endif
