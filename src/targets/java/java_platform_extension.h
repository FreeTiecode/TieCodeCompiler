#ifndef TIECODEC_TARGETS_JAVA_JAVA_PLATFORM_EXTENSION_H
#define TIECODEC_TARGETS_JAVA_JAVA_PLATFORM_EXTENSION_H

#include <string>
#include <vector>

#include "java_component.h"
#include "target.h"

namespace tiecodec {

class ClassInfo;
class ResourceBinder;

class JavaApplicationModel {
public:
  virtual ~JavaApplicationModel() = default;

  virtual std::string ApplicationBase(const ClassInfo& klass) const {
    (void)klass;
    return {};
  }

  virtual std::string CreateCompletedMethod() const {
    return "创建完毕";
  }

  virtual bool IsEntryClass(const ClassInfo& klass) const {
    (void)klass;
    return false;
  }
};

class JavaComponentModel {
public:
  virtual ~JavaComponentModel() = default;

  virtual std::string ComponentContextType() const {
    return {};
  }

  virtual std::string ComponentInitMethod() const {
    return {};
  }

  virtual std::string ComponentCreateMethod() const {
    return {};
  }

  virtual bool IsComponent(const ClassInfo& klass) const {
    (void)klass;
    return false;
  }

  virtual bool IsComponentContainer(const ClassInfo& klass) const {
    (void)klass;
    return false;
  }

  virtual bool IsComponentField(const Member& member) const {
    (void)member;
    return false;
  }

  virtual std::vector<ComponentField> CollectComponents(const ClassInfo& klass) const {
    (void)klass;
    return {};
  }

  virtual std::string ComponentReturnType(const ClassInfo& klass) const {
    (void)klass;
    return {};
  }

  virtual std::string ComponentAddMethod() const {
    return "添加组件";
  }

  virtual std::string ComponentSetLayoutMethod() const {
    return "setLayout";
  }

  virtual bool EmitsAutoSubscriptions(const ClassInfo& klass) const {
    (void)klass;
    return false;
  }
};

class JavaResourceModel {
public:
  virtual ~JavaResourceModel() = default;

  virtual std::string ResourceNamespace() const {
    return {};
  }

  virtual ResourceBinder* Resources() const {
    return nullptr;
  }

  virtual bool IsResourceValueRule(const std::string& rule) const {
    (void)rule;
    return false;
  }
};

class JavaPlatformExtension : public PlatformExtension,
                              public JavaApplicationModel,
                              public JavaComponentModel,
                              public JavaResourceModel {};

struct JavaPlatformServices {
  const JavaApplicationModel& application;
  const JavaComponentModel& components;
  const JavaResourceModel& resources;

  std::string ApplicationBase(const ClassInfo& klass) const {
    return application.ApplicationBase(klass);
  }

  std::string CreateCompletedMethod() const {
    return application.CreateCompletedMethod();
  }

  bool IsEntryClass(const ClassInfo& klass) const {
    return application.IsEntryClass(klass);
  }

  std::string ComponentContextType() const {
    return components.ComponentContextType();
  }

  std::string ComponentInitMethod() const {
    return components.ComponentInitMethod();
  }

  std::string ComponentCreateMethod() const {
    return components.ComponentCreateMethod();
  }

  bool IsComponent(const ClassInfo& klass) const {
    return components.IsComponent(klass);
  }

  bool IsComponentContainer(const ClassInfo& klass) const {
    return components.IsComponentContainer(klass);
  }

  bool IsComponentField(const Member& member) const {
    return components.IsComponentField(member);
  }

  std::vector<ComponentField> CollectComponents(const ClassInfo& klass) const {
    return components.CollectComponents(klass);
  }

  std::string ComponentReturnType(const ClassInfo& klass) const {
    return components.ComponentReturnType(klass);
  }

  std::string ComponentAddMethod() const {
    return components.ComponentAddMethod();
  }

  std::string ComponentSetLayoutMethod() const {
    return components.ComponentSetLayoutMethod();
  }

  bool EmitsAutoSubscriptions(const ClassInfo& klass) const {
    return components.EmitsAutoSubscriptions(klass);
  }

  std::string ResourceNamespace() const {
    return resources.ResourceNamespace();
  }

  ResourceBinder* Resources() const {
    return resources.Resources();
  }

  bool IsResourceValueRule(const std::string& rule) const {
    return resources.IsResourceValueRule(rule);
  }
};

}

#endif
