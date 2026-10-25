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

  virtual std::string LocalName(const std::string& name) const {
    return name;
  }

  virtual std::string AsyncContextName(const std::string& source_class_name,
                                       const std::string& member_name) const {
    return source_class_name + "_" + member_name + "_异步上下文";
  }

  virtual std::string AsyncMethodName(const Member& member) const {
    return MemberName(member) + "_异步";
  }

  virtual bool LoadStableNames(const std::string& path) {
    (void)path;
    return false;
  }

  virtual bool SaveNames(const std::string& path) const {
    (void)path;
    return false;
  }
};

}

#endif
