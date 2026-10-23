#ifndef TIECODEC_BACKEND_TYPE_MAPPER_H
#define TIECODEC_BACKEND_TYPE_MAPPER_H

#include <string>

#include "symbols.h"

namespace tiecodec {

class TypeMapper {
public:
  virtual ~TypeMapper() = default;

  virtual std::string MapType(const std::string& type_name,
                              const ClassInfo* context_class) const = 0;

  virtual bool IsNativeType(const std::string& type_name, const ClassInfo* context_class) const = 0;
};

}  // namespace tiecodec

#endif
