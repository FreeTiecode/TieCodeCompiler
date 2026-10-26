#include "register_extensions.h"

#include "android_annotations.h"
#include "android_requirements.h"
#include "common_annotations.h"
#include "cxx_annotations.h"
#include "cxx_language_factories.h"
#include "cxx_requirements.h"
#include "java_annotations.h"
#include "java_requirements.h"
#include "jvm_requirements.h"

namespace tiecodec {

bool RegisterBuiltinExtensions(ExtensionRegistry& registry, std::string* error) {
  RegisterCommonAnnotations(registry);
  RegisterJavaAnnotations(registry);
  RegisterCxxAnnotations(registry);
  RegisterAndroidAnnotations(registry);
  RegisterJavaRequirements(registry.requirements());
  RegisterCxxRequirements(registry.requirements());
  RegisterAndroidRequirements(registry.requirements());
  RegisterJvmRequirements(registry.requirements());
  RegisterCxxLanguageFactories(BuiltinLanguageFactories());
  if (!registry.ok()) {
    if (error != nullptr)
      *error = registry.error();
    return false;
  }
  return true;
}

}
