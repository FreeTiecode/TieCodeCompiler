#ifndef TIECODEC_BACKEND_JAVA_TYPE_MAPPER_H
#define TIECODEC_BACKEND_JAVA_TYPE_MAPPER_H

#include <string>

#include "naming.h"
#include "symbols.h"
#include "type_mapper.h"

namespace tiecodec {

class JavaTypeMapper : public TypeMapper {
public:
  explicit JavaTypeMapper(const SymbolTable& symbols, const NamingScheme* names = nullptr)
      : symbols_(symbols), names_(names) {}

  std::string MapType(const std::string& type_name, const ClassInfo* context_class) const override;
  bool IsNativeType(const std::string& type_name, const ClassInfo* context_class) const override;

private:
  const SymbolTable& symbols_;
  const NamingScheme* names_ = nullptr;
};

}  // namespace tiecodec

#endif
