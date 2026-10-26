#ifndef TIECODEC_TARGETS_CXX_CXX_TYPE_MAPPER_H
#define TIECODEC_TARGETS_CXX_CXX_TYPE_MAPPER_H

#include <string>
#include <vector>

#include "naming.h"
#include "symbols.h"
#include "type_mapper.h"

namespace tiecodec {

struct CxxMappedType {
  std::string text;
  bool is_primitive = false;
  bool is_object = false;
  std::string wrapper;
};

class CxxTypeMapper : public TypeMapper {
public:
  CxxTypeMapper(const SymbolTable& symbols, const NamingScheme* names);

  std::string MapType(const std::string& type_name, const ClassInfo* context_class) const override;

  bool IsNativeType(const std::string& type_name, const ClassInfo* context_class) const override;

  CxxMappedType Describe(const std::string& type_name, const ClassInfo* context_class) const;

  std::string WrapperFor(const std::string& type_name, const ClassInfo* context_class) const;

private:
  std::string MapTypeInner(const std::string& type_name, const ClassInfo* context_class,
                           bool box_objects) const;

  const SymbolTable& symbols_;
  const NamingScheme* names_ = nullptr;
};

std::vector<std::string> CxxSplitTypeArguments(const std::string& text);

std::string CxxBaseTypeName(const std::string& type_name);

}

#endif
