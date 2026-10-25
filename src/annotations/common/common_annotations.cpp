#include "common_annotations.h"

#include <memory>

#include "annotation_context.h"
#include "constants.h"
#include "extension_registry.h"
#include "language_constants.h"

namespace tiecodec {
namespace {

AnnotationMetadata ApplyMetadata(std::string name, unsigned levels,
                                 std::vector<ParamRule> positional = {},
                                 std::vector<ParamRule> named = {}) {
  return MakeMetadata(std::move(name), levels, Phases(AnnotationPhase::kApply),
                      std::move(positional), std::move(named));
}

AnnotationMetadata DeclareMetadata(std::string name, unsigned levels,
                                   std::vector<ParamRule> positional = {},
                                   std::vector<ParamRule> named = {}) {
  return MakeMetadata(std::move(name), levels, Phases(AnnotationPhase::kDeclare),
                      std::move(positional), std::move(named));
}

ClassFacts& ClassFactsOf(const DeclRef& declaration, DeclareContext& context) {
  return context.semantics.ClassFactsFor(declaration.klass);
}

MemberFacts& MemberFactsOf(const DeclRef& declaration, DeclareContext& context) {
  return context.semantics.MemberFactsFor(declaration.member);
}

}

AnnotationMetadata CompileConditionAnnotation::CreateMetadata() {
  return MakeMetadata(kCompileConditionAnnotation, kAnyMember, Phases(AnnotationPhase::kCondition),
                      {{kArgumentParameter, ParamKind::kCondition}});
}

AnnotationMetadata ForcedOutputAnnotation::CreateMetadata() {
  return ApplyMetadata("强制输出", kAnyMember, {}, {BoolParam()});
}

void ForcedOutputAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                   ApplyContext& context) const {
  const std::string value = annotation.TextArgument(kArgumentParameter);
  if (value.empty() || !IsTruthyLiteral(value))
    return;
  if (declaration.level == AnnotationLevel::kClass)
    context.plan.ForceClass(declaration.owner);
  else
    context.plan.ForceMember(declaration.owner, declaration.member);
}

AnnotationMetadata GlobalClassAnnotation::CreateMetadata() {
  return DeclareMetadata("全局类", kLevelClass);
}

void GlobalClassAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                    DeclareContext& context) const {
  (void)annotation;
  ClassFactsOf(declaration, context).global_class = true;
}

AnnotationMetadata GlobalBaseClassAnnotation::CreateMetadata() {
  return DeclareMetadata("全局基础类", kLevelClass);
}

void GlobalBaseClassAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                        DeclareContext& context) const {
  (void)annotation;
  ClassFactsOf(declaration, context).global_base = true;
}

AnnotationMetadata NoOutputAnnotation::CreateMetadata() {
  return DeclareMetadata("禁止输出", kLevelClass);
}

void NoOutputAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                 DeclareContext& context) const {
  (void)annotation;
  ClassFactsOf(declaration, context).forbid_output = true;
}

AnnotationMetadata NoInstantiationAnnotation::CreateMetadata() {
  return DeclareMetadata("禁止创建对象", kLevelClass);
}

void NoInstantiationAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                        DeclareContext& context) const {
  (void)annotation;
  ClassFactsOf(declaration, context).forbid_new = true;
}

AnnotationMetadata NoInheritAnnotation::CreateMetadata() {
  return DeclareMetadata("禁止继承", kLevelClass);
}

void NoInheritAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                  DeclareContext& context) const {
  (void)annotation;
  ClassFactsOf(declaration, context).forbid_inherit = true;
}

AnnotationMetadata NoPointerAnnotation::CreateMetadata() {
  return DeclareMetadata("禁用指针", kLevelClass);
}

void NoPointerAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                  DeclareContext& context) const {
  (void)annotation;
  ClassFactsOf(declaration, context).disable_pointer = true;
}

AnnotationMetadata DeprecatedAnnotation::CreateMetadata() {
  return DeclareMetadata("废弃使用", kAnyMember, {TextParam()});
}

void DeprecatedAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                   DeclareContext& context) const {
  (void)annotation;
  if (declaration.level == AnnotationLevel::kClass)
    ClassFactsOf(declaration, context).deprecated = true;
}

AnnotationMetadata ExternalBaseClassAnnotation::CreateMetadata() {
  return DeclareMetadata("外部基础类", kLevelClass, {TextParam()});
}

void ExternalBaseClassAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                          DeclareContext& context) const {
  ClassFactsOf(declaration, context).external_base = annotation.TextArgument(kArgumentParameter);
  if (!declaration.klass->base_name.empty())
    context.diagnostics.Error(context.unit.file, declaration.klass->base_position,
                              "类指定外部基础类之后不能继承父类", "parse.external-base.inherit");
}

AnnotationMetadata VirtualMethodAnnotation::CreateMetadata() {
  return MakeMetadata("虚拟方法", kLevelMethod,
                      AnnotationPhase::kDeclare | AnnotationPhase::kApply);
}

void VirtualMethodAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                      DeclareContext& context) const {
  (void)annotation;
  MemberFacts& facts = MemberFactsOf(declaration, context);
  facts.is_virtual = true;
  facts.force_use = true;
}

void VirtualMethodAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                    ApplyContext& context) const {
  (void)annotation;
  context.plan.ForceMember(declaration.owner, declaration.member);
}

AnnotationMetadata VirtualEventAnnotation::CreateMetadata() {
  return DeclareMetadata("虚拟事件", kLevelMethod);
}

void VirtualEventAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                     DeclareContext& context) const {
  (void)annotation;
  MemberFactsOf(declaration, context).is_virtual = true;
}

AnnotationMetadata PropertyReadAnnotation::CreateMetadata() {
  return MakeMetadata("属性读", kLevelMethod, 0);
}

AnnotationMetadata PropertyWriteAnnotation::CreateMetadata() {
  return MakeMetadata("属性写", kLevelMethod, 0);
}

void RegisterCommonAnnotations(ExtensionRegistry& registry) {
  registry.Register(std::make_unique<CompileConditionAnnotation>());
  registry.Register(std::make_unique<ForcedOutputAnnotation>());
  registry.Register(std::make_unique<GlobalClassAnnotation>());
  registry.Register(std::make_unique<GlobalBaseClassAnnotation>());
  registry.Register(std::make_unique<NoOutputAnnotation>());
  registry.Register(std::make_unique<NoInstantiationAnnotation>());
  registry.Register(std::make_unique<NoInheritAnnotation>());
  registry.Register(std::make_unique<NoPointerAnnotation>());
  registry.Register(std::make_unique<DeprecatedAnnotation>());
  registry.Register(std::make_unique<ExternalBaseClassAnnotation>());
  registry.Register(std::make_unique<VirtualMethodAnnotation>());
  registry.Register(std::make_unique<VirtualEventAnnotation>());
  registry.Register(std::make_unique<PropertyReadAnnotation>());
  registry.Register(std::make_unique<PropertyWriteAnnotation>());
}

}
