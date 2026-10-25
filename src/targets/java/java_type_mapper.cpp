#include "java_type_mapper.h"

#include <vector>

namespace tiecodec {
namespace {

std::string BaseTypeName(const std::string& type_name) {
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
    if (c == '[')
      break;
    if (c == '?')
      break;
    result.push_back(c);
  }
  return result;
}

std::string SimpleName(const std::string& qualified_name) {
  const std::size_t dot = qualified_name.rfind('.');
  return dot == std::string::npos ? qualified_name : qualified_name.substr(dot + 1);
}

std::string BoxPrimitive(const std::string& type) {
  if (type == "boolean")
    return "Boolean";
  if (type == "int")
    return "Integer";
  if (type == "long")
    return "Long";
  if (type == "float")
    return "Float";
  if (type == "double")
    return "Double";
  if (type == "byte")
    return "Byte";
  if (type == "short")
    return "Short";
  if (type == "char")
    return "Character";
  return type;
}

}

namespace {

std::vector<std::string> SplitTopLevelArguments(const std::string& text) {
  std::vector<std::string> arguments;
  std::string current;
  int depth = 0;
  for (char c : text) {
    if (c == '<')
      ++depth;
    if (c == '>')
      --depth;
    if (c == ',' && depth == 0) {
      arguments.push_back(current);
      current.clear();
      continue;
    }
    current.push_back(c);
  }
  if (!current.empty())
    arguments.push_back(current);
  return arguments;
}

}

std::string JavaTypeMapper::MapType(const std::string& type_name,
                                    const ClassInfo* context_class) const {
  if (type_name.empty())
    return "Object";
  const std::string base = BaseTypeName(type_name);
  const ClassInfo* klass = symbols_.Resolve(type_name, context_class);
  std::string result;
  if (klass != nullptr && klass->HasExternalType("java")) {
    result = klass->ExternalType("java");
  } else if (klass != nullptr) {
    if (!klass->output_name.empty()) {
      result = klass->output_name;
    } else if (names_ != nullptr) {
      result = names_->ClassName(*klass);
    } else {
      result = SimpleName(klass->qualified_name);
    }
  } else {
    result = base;
  }
  const std::size_t open = type_name.find('<');
  const std::size_t close = type_name.rfind('>');
  if (open != std::string::npos && close != std::string::npos && close > open) {
    const std::vector<std::string> arguments =
        SplitTopLevelArguments(type_name.substr(open + 1, close - open - 1));
    std::string mapped;
    for (std::size_t i = 0; i < arguments.size(); ++i) {
      if (i > 0)
        mapped += ", ";
      mapped += BoxPrimitive(MapType(arguments[i], context_class));
    }
    if (!mapped.empty())
      result += "<" + mapped + ">";
  }

  const std::size_t bracket = type_name.find('[');
  if (bracket != std::string::npos) {
    for (std::size_t i = bracket; i < type_name.size(); ++i) {
      if (type_name[i] == '[')
        result += "[]";
    }
  }
  return result;
}

bool JavaTypeMapper::IsNativeType(const std::string& type_name,
                                  const ClassInfo* context_class) const {
  const ClassInfo* klass = symbols_.Resolve(type_name, context_class);
  return klass != nullptr && klass->HasExternalType("java");
}

}
