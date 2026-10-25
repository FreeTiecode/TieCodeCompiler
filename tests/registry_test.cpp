#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "common_annotations.h"
#include "extension_registry.h"
#include "register_extensions.h"
#include "register_targets.h"
#include "requirement.h"
#include "target_registry.h"

namespace tiecodec {
namespace test {
namespace {

TargetKey AndroidTarget() {
  return TargetKey{"android", "java"};
}

TargetKey JvmTarget() {
  return TargetKey{"jvm", "java"};
}

bool HasRequirement(const std::vector<const PlatformRequirement*>& requirements,
                    const std::string& id) {
  return std::any_of(
      requirements.begin(), requirements.end(),
      [&id](const PlatformRequirement* requirement) { return requirement->Metadata().id == id; });
}

}

TEST(RegistryTest, RegistersBuiltinTargets) {
  TargetRegistry registry;
  std::string error;
  ASSERT_TRUE(RegisterBuiltinTargets(registry, &error)) << error;
  ASSERT_NE(registry.Find("android", "java"), nullptr);
  ASSERT_NE(registry.Find("jvm", "java"), nullptr);
  EXPECT_EQ(registry.Find("jvm", "cpp"), nullptr);
  EXPECT_EQ(registry.DefaultLanguage("android"), "java");
  EXPECT_EQ(registry.DefaultLanguage("jvm"), "java");

  const std::vector<std::string> platforms = registry.platforms();
  EXPECT_NE(std::find(platforms.begin(), platforms.end(), "android"), platforms.end());
  EXPECT_NE(std::find(platforms.begin(), platforms.end(), "jvm"), platforms.end());
}

TEST(RegistryTest, RejectsDuplicateRegistrations) {
  TargetRegistry targets;
  std::string error;
  ASSERT_TRUE(RegisterBuiltinTargets(targets, &error)) << error;
  const TargetDescriptor* existing = targets.Find("android", "java");
  ASSERT_NE(existing, nullptr);
  TargetDescriptor duplicate = *existing;
  EXPECT_FALSE(targets.Register(std::move(duplicate)));

  ExtensionRegistry extensions;
  ASSERT_TRUE(RegisterBuiltinExtensions(extensions, &error)) << error;
  EXPECT_FALSE(extensions.Register(std::make_unique<CompileConditionAnnotation>()));
  EXPECT_FALSE(extensions.ok());
}

TEST(RegistryTest, FiltersAnnotationsByTarget) {
  std::string error;
  ExtensionRegistry first;
  ASSERT_TRUE(RegisterBuiltinExtensions(first, &error)) << error;
  ExtensionRegistry second;
  ASSERT_TRUE(RegisterBuiltinExtensions(second, &error)) << error;

  EXPECT_NE(first.annotations().Find("编译条件", AndroidTarget()), nullptr);
  EXPECT_NE(first.annotations().Find("编译条件", JvmTarget()), nullptr);
  EXPECT_NE(first.annotations().Find("隐藏", JvmTarget()), nullptr);
  EXPECT_NE(first.annotations().Find("安卓资源", AndroidTarget()), nullptr);
  EXPECT_EQ(first.annotations().Find("安卓资源", JvmTarget()), nullptr);
  EXPECT_EQ(first.annotations().Find("不存在的注解", AndroidTarget()), nullptr);
  EXPECT_EQ(first.annotations().names().size(), second.annotations().names().size());
}

TEST(RegistryTest, RegistersRequirementsPerTarget) {
  ExtensionRegistry registry;
  std::string error;
  ASSERT_TRUE(RegisterBuiltinExtensions(registry, &error)) << error;

  const std::vector<const PlatformRequirement*> android =
      registry.requirements().Matching(AndroidTarget());
  const std::vector<const PlatformRequirement*> jvm = registry.requirements().Matching(JvmTarget());

  EXPECT_TRUE(HasRequirement(android, "android.member.窗口组件.创建完毕"));
  EXPECT_FALSE(HasRequirement(jvm, "android.member.窗口组件.创建完毕"));
  EXPECT_TRUE(HasRequirement(jvm, "jvm.member.控制台.输出"));
  EXPECT_TRUE(HasRequirement(jvm, "java.type.文本"));
  EXPECT_TRUE(HasRequirement(android, "java.type.文本"));
}

}

}
