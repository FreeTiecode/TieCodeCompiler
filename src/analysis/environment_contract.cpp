#include "environment_contract.h"

#include <algorithm>
#include <string>

#include "symbol_resolution.h"

namespace tiecodec {
namespace {

std::string SimpleName(const std::string& qualified_name) {
  const std::size_t dot = qualified_name.rfind('.');
  return dot == std::string::npos ? qualified_name : qualified_name.substr(dot + 1);
}

bool HasDirectMember(const ClassDecl& declaration, const std::string& name) {
  for (const Member& member : declaration.members) {
    if (member.name == name)
      return true;
  }
  return false;
}

}  // namespace

void ValidateEnvironment(const EnvironmentContract& contract, const SymbolTable& symbols,
                         Diagnostics* diagnostics) {
  for (const RequiredType& required : contract.RequiredTypes()) {
    bool found = false;
    for (const std::string& qualified_name : symbols.class_names()) {
      if (SimpleName(qualified_name) != required.name)
        continue;
      found = true;
      break;
    }
    if (found)
      continue;
    diagnostics->Fatal(std::string("当前编译环境中缺少编译器必要类型定义类 \"") + required.name +
                           "\"，此为致命错误",
                       "enter.basic-type.missed");
  }
  for (const RequiredMember& required : contract.RequiredMembers()) {
    const ClassInfo* owner = nullptr;
    for (const std::string& qualified_name : symbols.class_names()) {
      if (SimpleName(qualified_name) != required.class_name)
        continue;
      owner = symbols.Find(qualified_name);
      break;
    }
    if (owner == nullptr || owner->declaration == nullptr)
      continue;
    if (HasDirectMember(*owner->declaration, required.member_name))
      continue;
    diagnostics->Fatal(std::string("类\"") + required.class_name + "\"中缺少成员\"" +
                           required.member_name + "\"，编译环境缺失，此为致命错误",
                       "enter.member.missed");
  }
}

}  // namespace tiecodec
