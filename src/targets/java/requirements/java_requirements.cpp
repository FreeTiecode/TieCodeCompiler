#include "java_requirements.h"

#include "java_language_constants.h"

#include <memory>
#include <string>

#include "requirement.h"
#include "requirement_registry.h"
#include "target_key.h"

namespace tiecodec {
namespace {

RequirementMetadata TypeRequirement(const std::string& name) {
  RequirementMetadata metadata;
  metadata.id = "java.type." + name;
  metadata.kind = RequirementKind::kType;
  metadata.type_name = name;
  metadata.target = LanguageTarget(kJavaLanguageName);
  return metadata;
}

RequirementMetadata MemberRequirement(const std::string& owner, const std::string& member,
                                      bool seed_use = true, bool virtual_hook = false) {
  RequirementMetadata metadata;
  metadata.id = "java.member." + owner + "." + member;
  metadata.kind = RequirementKind::kMember;
  metadata.owner_class = owner;
  metadata.member_name = member;
  metadata.target = LanguageTarget(kJavaLanguageName);
  metadata.seed_use = seed_use;
  metadata.virtual_hook = virtual_hook;
  return metadata;
}

void AddType(RequirementRegistry& registry, const std::string& name) {
  registry.Register(std::make_unique<DeclarativeRequirement>(TypeRequirement(name)));
}

void AddMember(RequirementRegistry& registry, const std::string& owner, const std::string& member,
               bool seed_use = true, bool virtual_hook = false) {
  registry.Register(std::make_unique<DeclarativeRequirement>(
      MemberRequirement(owner, member, seed_use, virtual_hook)));
}

}

void RegisterJavaRequirements(RequirementRegistry& registry) {
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

  AddMember(registry, "遍历器", "还有下一个");
  AddMember(registry, "遍历器", "下一个");
  AddMember(registry, "键值表条目", "键");
  AddMember(registry, "键值表条目", "值");
  AddMember(registry, "异步调度器", "创建单线程调度器", false);
  AddMember(registry, "异步调度器", "提交到全局调度器运行", false);
  AddMember(registry, "异步调度器", "结束提交到全局调度器运行", false);
  AddMember(registry, "异步调度器", "提交到调度器运行", false);
  AddMember(registry, "异步调度器", "结束提交到调度器运行", false);
  AddMember(registry, "异步调度器", "提交到主线程运行", false);
  AddMember(registry, "异步调度器", "结束提交到主线程运行", false);
}

}
