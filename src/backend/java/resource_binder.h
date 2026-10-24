#ifndef TIECODEC_BACKEND_JAVA_RESOURCE_BINDER_H
#define TIECODEC_BACKEND_JAVA_RESOURCE_BINDER_H

#include <string>

#include "symbols.h"

namespace tiecodec {

class ResourceBinder {
public:
  virtual ~ResourceBinder() = default;

  virtual bool IsResourceField(const ClassInfo& owner, const Member& member) const = 0;

  virtual std::string FieldType(const ClassInfo& owner, const Member& member) const = 0;

  virtual std::string BindField(const ClassInfo& owner, const Member& member) = 0;

  virtual std::string ResourceImport(const ClassInfo& owner) const = 0;
};

}

#endif
