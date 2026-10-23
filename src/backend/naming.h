#ifndef TIECODEC_BACKEND_NAMING_H
#define TIECODEC_BACKEND_NAMING_H

#include <string>

#include "symbols.h"

namespace tiecodec {

class NamingScheme {
public:
  virtual ~NamingScheme() = default;
  virtual std::string ClassName(const ClassInfo& klass) const = 0;
  virtual std::string MemberName(const Member& member) const = 0;
  virtual std::string BaseName(const ClassInfo& klass) const = 0;
  virtual std::string Package(const ClassInfo& klass) const = 0;
  virtual std::string ImportPath(const ClassInfo& klass) const = 0;

  virtual std::string GeneratedName(const std::string& hint, int index) const = 0;

  /// Output package of the generated resource class (R).
  virtual std::string RNamespace() const {
    return {};
  }

  /// Name of a local variable or parameter in the output language.
  virtual std::string LocalName(const std::string& name) const {
    return name;
  }

  /// Load a stable name mapping file (friendly name modes).
  virtual bool LoadStableNames(const std::string& path) {
    (void)path;
    return false;
  }

  /// Save the current name mapping file.
  virtual bool SaveNames(const std::string& path) const {
    (void)path;
    return false;
  }
};

}  // namespace tiecodec

#endif
