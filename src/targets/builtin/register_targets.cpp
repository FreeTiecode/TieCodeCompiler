#include "register_targets.h"

#include <memory>
#include <string>
#include <utility>

#include "android_constants.h"
#include "android_java_extension.h"
#include "android_platform_backend.h"
#include "cxx_language_backend.h"
#include "cxx_language_constants.h"
#include "cxx_platform_backend.h"
#include "java_language_backend.h"
#include "java_language_constants.h"
#include "jvm_constants.h"
#include "jvm_java_extension.h"
#include "jvm_platform_backend.h"

namespace tiecodec {
namespace {

template <typename Platform, typename Extension, typename... Args>
TargetSession CreateJavaSession(std::shared_ptr<Extension> extension, Args&&... args) {
  TargetSession session;
  session.extension = extension;
  session.language = std::make_unique<JavaLanguageBackend>(extension);
  session.platform = std::make_unique<Platform>(std::forward<Args>(args)...);
  return session;
}

TargetDescriptor AndroidTarget() {
  TargetDescriptor descriptor;
  descriptor.key = TargetKey{kAndroidPlatformName, kJavaLanguageName};
  descriptor.display_name = kAndroidDisplayName;
  descriptor.entry_class_name = kAndroidEntryClassName;
  const std::string entry_class_name = descriptor.entry_class_name;
  descriptor.create = [entry_class_name](const GenerationInput& input) {
    auto extension = std::make_shared<AndroidJavaExtension>(
        input.symbols, input.config, input.semantics, input.effects, entry_class_name);
    return CreateJavaSession<AndroidPlatformBackend>(extension, input, extension);
  };
  return descriptor;
}

TargetDescriptor JvmTarget() {
  TargetDescriptor descriptor;
  descriptor.key = TargetKey{kJvmPlatformName, kJavaLanguageName};
  descriptor.display_name = kJvmDisplayName;
  descriptor.entry_class_name = kJvmEntryClassName;
  descriptor.entry_method = kJvmEntryMethodName;
  descriptor.create = [](const GenerationInput& input) {
    auto extension = std::make_shared<JvmJavaExtension>();
    return CreateJavaSession<JvmPlatformBackend>(extension, input);
  };
  return descriptor;
}

TargetDescriptor CxxTarget() {
  TargetDescriptor descriptor;
  descriptor.key = TargetKey{kCxxPlatformName, kCxxLanguageName};
  descriptor.display_name = kCxxDisplayName;
  descriptor.entry_class_name = kCxxEntryClassName;
  descriptor.entry_method = kCxxEntryMethodName;
  descriptor.create = [](const GenerationInput& input) {
    TargetSession session;
    session.language = std::make_unique<CxxLanguageBackend>();
    session.platform = std::make_unique<CxxPlatformBackend>(input);
    return session;
  };
  return descriptor;
}

}

bool RegisterBuiltinTargets(TargetRegistry& registry, std::string* error) {
  if (!registry.Register(AndroidTarget())) {
    if (error != nullptr)
      *error = "重复注册目标: android/java";
    return false;
  }
  if (!registry.Register(JvmTarget())) {
    if (error != nullptr)
      *error = "重复注册目标: jvm/java";
    return false;
  }
  if (!registry.Register(CxxTarget())) {
    if (error != nullptr)
      *error = "重复注册目标: cxx/cxx";
    return false;
  }
  return true;
}

}
