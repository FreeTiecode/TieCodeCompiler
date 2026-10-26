#include "cxx_type_mapper.h"

#include <map>
#include <string>
#include <vector>

#include "cxx_language_constants.h"
#include "internal/helpers.h"
#include "text_util.h"

namespace tiecodec {
namespace {

struct PrimitiveMapping {
  const char* alias;
  const char* wrapper;
};

const std::map<std::string, PrimitiveMapping>& PrimitiveTypes() {
  static const std::map<std::string, PrimitiveMapping> types = {
      {"整数", {"T_INT", "CTieInt"}},       {"长整数", {"T_LONG", "CTieLong"}},
      {"小数", {"T_DOUBLE", "CTieDouble"}}, {"单精度小数", {"T_FLOAT", "CTieFloat"}},
      {"逻辑型", {"T_BOOL", "CTieBool"}},   {"字符", {"T_CHAR", "CTieChar"}},
      {"字节", {"T_BYTE", "CTieByte"}},     {"文本", {"T_STRING", "CTieString"}},
  };
  return types;
}

}

std::string CxxBaseTypeName(const std::string& type_name) {
  std::string result;
  int depth = 0;
  for (char c : type_name) {
    if (c == '<') {
      ++depth;
      continue;
    }
    if (c == '>') {
      --depth;
      continue;
    }
    if (depth > 0)
      continue;
    if (c == '[' || c == '?')
      break;
    result.push_back(c);
  }
  return Trim(result);
}

std::vector<std::string> CxxSplitTypeArguments(const std::string& text) {
  std::vector<std::string> arguments;
  std::string current;
  int depth = 0;
  for (char c : text) {
    if (c == '<')
      ++depth;
    if (c == '>')
      --depth;
    if (c == ',' && depth == 0) {
      arguments.push_back(Trim(current));
      current.clear();
      continue;
    }
    current.push_back(c);
  }
  if (!Trim(current).empty())
    arguments.push_back(Trim(current));
  return arguments;
}

CxxTypeMapper::CxxTypeMapper(const SymbolTable& symbols, const NamingScheme* names)
    : symbols_(symbols), names_(names) {}

std::string CxxTypeMapper::MapTypeInner(const std::string& type_name,
                                        const ClassInfo* context_class, bool box_objects) const {
  const std::string trimmed = Trim(type_name);
  if (trimmed.empty())
    return std::string(kCxxSharedPtr) + "<" + kCxxObjectClass + ">";

  const detail::TypeShape shape = detail::ParseTypeShape(trimmed);
  const std::string base = Trim(shape.base);
  const auto primitive = PrimitiveTypes().find(base);
  if (primitive != PrimitiveTypes().end() && shape.dimensions == 0)
    return primitive->second.alias;

  const ClassInfo* klass = symbols_.Resolve(base, context_class);
  std::string result;
  if (klass != nullptr) {
    if (klass->HasExternalType(kCxxTypeAliasKey)) {
      result = klass->ExternalType(kCxxTypeAliasKey);
    } else if (names_ != nullptr) {
      result = names_->ClassName(*klass);
    } else {
      const std::size_t dot = klass->qualified_name.rfind('.');
      result =
          dot == std::string::npos ? klass->qualified_name : klass->qualified_name.substr(dot + 1);
    }
  } else {
    result = base;
  }

  const std::size_t open = trimmed.find('<');
  const std::size_t close = trimmed.rfind('>');
  if (open != std::string::npos && close != std::string::npos && close > open) {
    const std::vector<std::string> arguments =
        CxxSplitTypeArguments(trimmed.substr(open + 1, close - open - 1));
    std::string mapped;
    for (std::size_t i = 0; i < arguments.size(); ++i) {
      if (i > 0)
        mapped += ", ";
      mapped += MapTypeInner(arguments[i], context_class, false);
    }
    if (!mapped.empty())
      result += "<" + mapped + ">";
  }

  if (shape.dimensions > 0) {
    for (int i = 0; i < shape.dimensions; ++i) {
      result = std::string(kCxxSharedPtr) + "<" + kCxxArrayClass + "<" + result + ">>";
    }
    return result;
  }
  if (klass == nullptr)
    return result;
  if (klass->disable_pointer)
    return result;
  if (klass->HasExternalType(kCxxTypeAliasKey) && klass->declaration != nullptr &&
      !klass->declaration->generic_parameters.empty()) {
    return std::string(kCxxSharedPtr) + "<" + result + ">";
  }
  if (box_objects)
    return std::string(kCxxSharedPtr) + "<" + result + ">";
  return result;
}

std::string CxxTypeMapper::MapType(const std::string& type_name,
                                   const ClassInfo* context_class) const {
  return MapTypeInner(type_name, context_class, true);
}

bool CxxTypeMapper::IsNativeType(const std::string& type_name,
                                 const ClassInfo* context_class) const {
  const ClassInfo* klass = symbols_.Resolve(CxxBaseTypeName(Trim(type_name)), context_class);
  return klass != nullptr && klass->HasExternalType(kCxxTypeAliasKey);
}

CxxMappedType CxxTypeMapper::Describe(const std::string& type_name,
                                      const ClassInfo* context_class) const {
  CxxMappedType mapped;
  const std::string trimmed = Trim(type_name);
  const std::string base = CxxBaseTypeName(trimmed);
  const auto primitive = PrimitiveTypes().find(base);
  if (primitive != PrimitiveTypes().end() && trimmed.find('[') == std::string::npos) {
    mapped.text = primitive->second.alias;
    mapped.is_primitive = true;
    mapped.wrapper = primitive->second.wrapper;
    return mapped;
  }
  if (base == "对象" || trimmed.empty()) {
    mapped.text = std::string(kCxxSharedPtr) + "<" + kCxxObjectClass + ">";
    mapped.is_object = true;
    mapped.wrapper = std::string(kCxxSharedPtr) + "<" + kCxxObjectClass + ">";
    return mapped;
  }
  mapped.text = MapType(trimmed, context_class);
  return mapped;
}

std::string CxxTypeMapper::WrapperFor(const std::string& type_name,
                                      const ClassInfo* context_class) const {
  const CxxMappedType mapped = Describe(type_name, context_class);
  if (!mapped.wrapper.empty() && mapped.wrapper == mapped.text)
    return mapped.text;
  if (mapped.text.empty())
    return std::string(kCxxSharedPtr) + "<" + kCxxObjectClass + ">";
  return std::string(kCxxSharedPtr) + "<" + mapped.text + ">";
}

}
