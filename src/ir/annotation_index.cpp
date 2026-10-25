#include "annotation_index.h"

#include <set>
#include <string>

namespace tiecodec {
namespace {

std::string StripGenerics(const std::string& name) {
  const std::size_t open = name.find('<');
  if (open == std::string::npos)
    return name;
  const std::size_t close = name.rfind('>');
  if (close == std::string::npos || close < open)
    return name;
  return name.substr(0, open) + name.substr(close + 1);
}

}

void BuildAnnotationSymbolIndex(const std::vector<Unit>& units, AnnotationSymbolIndex* index) {
  if (index == nullptr)
    return;
  index->class_names.clear();
  index->constant_names.clear();
  index->member_names.clear();
  index->class_constants.clear();
  index->class_members.clear();
  index->class_base.clear();
  index->class_package.clear();
  index->builtin_types = {"对象",       "整数",     "文本",       "长整数", "逻辑型",
                          "字符",       "字节",     "单精度小数", "小数",   "遍历器",
                          "键值表条目", "任意类型", "任何类型",   "类型",   "变体型"};
  for (const Unit& unit : units) {
    for (const ClassDecl& class_decl : unit.classes) {
      index->class_names.insert(class_decl.name);
      index->class_base[class_decl.name] = class_decl.base_name;
      index->class_package[class_decl.name] = unit.package_name;
      for (const Member& member : class_decl.members) {
        index->member_names.insert(member.name);
        index->class_members[class_decl.name].insert(member.name);
        if (member.kind != MemberKind::kConstant)
          continue;
        index->constant_names.insert(member.name);
        index->class_constants[class_decl.name].insert(member.name);
      }
    }
  }
}

bool AnnotationSymbolIndex::ResolveSymbol(const std::string& raw_name) const {
  const std::string name = StripGenerics(raw_name);
  if (name.empty())
    return false;
  if (class_names.count(name) != 0 || builtin_types.count(name) != 0 ||
      constant_names.count(name) != 0) {
    return true;
  }
  const std::size_t dot = name.find('.');
  if (dot == std::string::npos)
    return false;
  const std::string class_name = name.substr(0, dot);
  const auto found = class_constants.find(class_name);
  if (found == class_constants.end())
    return false;
  const std::string rest = name.substr(dot + 1);
  if (found->second.count(rest) != 0)
    return true;
  const std::size_t last_dot = rest.rfind('.');
  if (last_dot != std::string::npos && found->second.count(rest.substr(last_dot + 1)) != 0)
    return true;
  return false;
}

bool AnnotationSymbolIndex::ResolveTypeName(const std::string& raw_name) const {
  const std::string name = StripGenerics(raw_name);
  if (name.empty())
    return false;
  return class_names.count(name) != 0 || builtin_types.count(name) != 0;
}

bool AnnotationSymbolIndex::ResolveMemberName(const std::string& raw_name,
                                              const std::string& owner_class) const {
  const std::string name = StripGenerics(raw_name);
  if (name.empty())
    return false;
  const std::size_t dot = name.find('.');
  if (dot != std::string::npos) {
    const std::string class_name = name.substr(0, dot);
    const auto found = class_members.find(class_name);
    if (found != class_members.end()) {
      const std::string rest = name.substr(dot + 1);
      if (found->second.count(rest) != 0)
        return true;
      const std::size_t last_dot = rest.rfind('.');
      if (last_dot != std::string::npos && found->second.count(rest.substr(last_dot + 1)) != 0)
        return true;
    }
    return false;
  }
  if (!owner_class.empty()) {
    std::string current = owner_class;
    std::set<std::string> visited;
    while (!current.empty() && visited.insert(current).second) {
      const auto found = class_members.find(current);
      if (found != class_members.end() && found->second.count(name) != 0)
        return true;
      const auto base = class_base.find(current);
      if (base == class_base.end() || base->second.empty())
        break;
      current = StripGenerics(base->second);
    }
    return false;
  }
  if (member_names.count(name) != 0 || class_names.count(name) != 0)
    return true;
  return false;
}

bool AnnotationSymbolIndex::ClassChainContains(const std::string& type_name,
                                               const std::string& target) const {
  std::string current = StripGenerics(type_name);
  std::set<std::string> visited;
  while (!current.empty() && visited.insert(current).second) {
    if (current == target)
      return true;
    const auto found = class_base.find(current);
    if (found == class_base.end() || found->second.empty())
      break;
    current = StripGenerics(found->second);
  }
  return false;
}

std::string AnnotationSymbolIndex::QualifiedNameOf(const std::string& class_name) const {
  const auto found = class_package.find(class_name);
  if (found == class_package.end() || found->second.empty())
    return class_name;
  return found->second + "." + class_name;
}

}
