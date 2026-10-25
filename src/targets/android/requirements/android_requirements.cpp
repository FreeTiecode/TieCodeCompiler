#include "android_requirements.h"

#include <memory>
#include <string>

#include "android_constants.h"
#include "requirement.h"
#include "requirement_registry.h"
#include "target_key.h"

namespace tiecodec {
namespace {

RequirementMetadata TypeRequirement(const std::string& name) {
  RequirementMetadata metadata;
  metadata.id = "android.type." + name;
  metadata.kind = RequirementKind::kType;
  metadata.type_name = name;
  metadata.target = PlatformTarget(kAndroidPlatformName);
  return metadata;
}

RequirementMetadata MemberRequirement(const std::string& owner, const std::string& member,
                                      bool seed_use = true, bool virtual_hook = false) {
  RequirementMetadata metadata;
  metadata.id = "android.member." + owner + "." + member;
  metadata.kind = RequirementKind::kMember;
  metadata.owner_class = owner;
  metadata.member_name = member;
  metadata.target = PlatformTarget(kAndroidPlatformName);
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

void RegisterAndroidRequirements(RequirementRegistry& registry) {
  AddType(registry, "安卓环境");
  AddType(registry, "安卓窗口");
  AddType(registry, "窗口组件");
  AddType(registry, "自定义组件");
  AddType(registry, "组件容器");
  AddType(registry, "服务");
  AddType(registry, "广播接收器");
  AddType(registry, "文本资源");
  AddType(registry, "主题资源");
  AddType(registry, "图片资源");
  AddType(registry, "应用");

  AddMember(registry, "窗口组件", "取安卓环境");
  AddMember(registry, "窗口组件", "创建完毕", false, true);
  AddMember(registry, "布局组件", "添加组件");
}

}
