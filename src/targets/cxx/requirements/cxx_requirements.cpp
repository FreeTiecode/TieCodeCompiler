#include "cxx_requirements.h"

#include <memory>
#include <string>

#include "cxx_language_constants.h"
#include "requirement.h"
#include "requirement_registry.h"
#include "target_key.h"

namespace tiecodec {
namespace {

RequirementMetadata TypeRequirement(const std::string& name) {
  RequirementMetadata metadata;
  metadata.id = "cxx.type." + name;
  metadata.kind = RequirementKind::kType;
  metadata.type_name = name;
  metadata.target = LanguageTarget(kCxxLanguageName);
  return metadata;
}

RequirementMetadata MemberRequirement(const std::string& owner, const std::string& member) {
  RequirementMetadata metadata;
  metadata.id = "cxx.member." + owner + "." + member;
  metadata.kind = RequirementKind::kMember;
  metadata.owner_class = owner;
  metadata.member_name = member;
  metadata.target = LanguageTarget(kCxxLanguageName);
  return metadata;
}

void AddType(RequirementRegistry& registry, const std::string& name) {
  registry.Register(std::make_unique<DeclarativeRequirement>(TypeRequirement(name)));
}

void AddMember(RequirementRegistry& registry, const std::string& owner, const std::string& member) {
  registry.Register(std::make_unique<DeclarativeRequirement>(MemberRequirement(owner, member)));
}

}

void RegisterCxxRequirements(RequirementRegistry& registry) {
  AddType(registry, "对象");
  AddType(registry, "文本");
  AddType(registry, "整数");
  AddType(registry, "长整数");
  AddType(registry, "逻辑型");
  AddType(registry, "字符");
  AddType(registry, "字节");
  AddType(registry, "单精度小数");
  AddType(registry, "小数");
  AddType(registry, "遍历器");
  AddType(registry, "键值表条目");
  AddType(registry, "异步调度器");
  AddMember(registry, "异步调度器", "创建单线程调度器");
}

}
