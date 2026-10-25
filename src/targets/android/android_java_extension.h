#ifndef TIECODEC_TARGETS_ANDROID_ANDROID_JAVA_EXTENSION_H
#define TIECODEC_TARGETS_ANDROID_ANDROID_JAVA_EXTENSION_H

#include <memory>
#include <string>
#include <vector>

#include "effect_store.h"
#include "java_platform_extension.h"
#include "semantic_model.h"
#include "symbols.h"
#include "target_config.h"

namespace tiecodec {

class AndroidResourceBinder;

class AndroidJavaExtension : public JavaPlatformExtension {
public:
  AndroidJavaExtension(const SymbolTable& symbols, const TargetConfig& config,
                       const SemanticModel& semantics, const EffectStore& effects,
                       std::string entry_class_name);
  ~AndroidJavaExtension() override;

  std::string ApplicationBase(const ClassInfo& klass) const override;
  std::string ComponentContextType() const override;
  std::string ComponentInitMethod() const override;
  std::string ComponentCreateMethod() const override;
  std::string CreateCompletedMethod() const override;
  std::string ResourceNamespace() const override;
  bool IsComponent(const ClassInfo& klass) const override;
  bool IsComponentContainer(const ClassInfo& klass) const override;
  bool IsEntryClass(const ClassInfo& klass) const override;
  bool IsComponentField(const Member& member) const override;
  std::vector<ComponentField> CollectComponents(const ClassInfo& klass) const override;
  std::string ComponentReturnType(const ClassInfo& klass) const override;
  std::string ComponentAddMethod() const override;
  std::string ComponentSetLayoutMethod() const override;
  bool EmitsAutoSubscriptions(const ClassInfo& klass) const override;
  ResourceBinder* Resources() const override;
  bool IsResourceValueRule(const std::string& rule) const override;

  const std::string& EntryClassName() const {
    return entry_class_name_;
  }

private:
  const SymbolTable& symbols_;
  const TargetConfig& config_;
  const SemanticModel& semantics_;
  const EffectStore& effects_;
  std::string entry_class_name_;
  std::unique_ptr<AndroidResourceBinder> resources_;
};

}

#endif
