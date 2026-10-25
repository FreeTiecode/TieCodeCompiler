#include "java_annotations.h"

#include <algorithm>
#include <memory>
#include <string>

#include "annotation_context.h"
#include "constants.h"
#include "extension_registry.h"
#include "java_effect_types.h"
#include "java_language_constants.h"

namespace tiecodec {
namespace {

bool IsValidJavaIdentifier(const std::string& name) {
  if (name.empty())
    return false;
  bool first = true;
  for (char raw : name) {
    const unsigned char c = static_cast<unsigned char>(raw);
    const bool ascii_alpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    const bool ascii_digit = c >= '0' && c <= '9';
    if (first) {
      if (!(ascii_alpha || c == '_' || c == '$' || c >= 0x80))
        return false;
      first = false;
      continue;
    }
    if (!(ascii_alpha || ascii_digit || c == '_' || c == '$' || c >= 0x80))
      return false;
  }
  return true;
}

AnnotationMetadata DeclareMetadata(std::string name, unsigned levels,
                                   std::vector<ParamRule> positional = {},
                                   std::vector<ParamRule> named = {}) {
  return MakeMetadata(std::move(name), levels, Phases(AnnotationPhase::kDeclare),
                      std::move(positional), std::move(named), LanguageTarget(kJavaLanguageName));
}

}

AnnotationMetadata OutputNameAnnotation::CreateMetadata() {
  return DeclareMetadata("输出名", kAllLevels, {TextParam()});
}

void OutputNameAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                   DeclareContext& context) const {
  const std::string value = annotation.TextArgument(kArgumentParameter);
  if (declaration.level == AnnotationLevel::kUnit) {
    if (value.find('.') != std::string::npos)
      context.semantics.UnitFactsFor(declaration.unit).output_package = value;
    return;
  }
  if (declaration.level == AnnotationLevel::kClass) {
    if (!IsValidJavaIdentifier(value)) {
      context.diagnostics.Error(context.unit.file, declaration.klass->position,
                                "类输出名不是合法的 Java 标识符: " + value, "enter.output-name");
      return;
    }
    context.semantics.ClassFactsFor(declaration.klass).output_name = value;
    return;
  }
  if (declaration.member != nullptr)
    context.semantics.MemberFactsFor(declaration.member).output_name = value;
}

AnnotationMetadata HiddenAnnotation::CreateMetadata() {
  return MakeMetadata("隐藏", kAnyMember,
                      Phases(AnnotationPhase::kDeclare) | Phases(AnnotationPhase::kApply), {}, {},
                      LanguageTarget(kJavaLanguageName));
}

void HiddenAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                               DeclareContext& context) const {
  (void)annotation;
  if (declaration.level == AnnotationLevel::kClass)
    context.semantics.ClassFactsFor(declaration.klass).hidden = true;
  else if (declaration.member != nullptr)
    context.semantics.MemberFactsFor(declaration.member).hidden = true;
}

void HiddenAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                             ApplyContext& context) const {
  (void)annotation;
  const void* node = declaration.member != nullptr ? static_cast<const void*>(declaration.member)
                                                   : static_cast<const void*>(declaration.klass);
  context.effects.Table<JavaModifiers>().Ensure(node).hidden = true;
}

AnnotationMetadata StaticAnnotation::CreateMetadata() {
  return MakeMetadata("静态", kLevelVariable | kLevelMethod, Phases(AnnotationPhase::kApply), {},
                      {}, LanguageTarget(kJavaLanguageName));
}

void StaticAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                             ApplyContext& context) const {
  (void)annotation;
  context.effects.Table<JavaModifiers>().Ensure(declaration.member).is_static = true;
}

AnnotationMetadata PrefixCodeAnnotation::CreateMetadata() {
  return MakeMetadata("前缀代码", kAnyMember, Phases(AnnotationPhase::kApply), {TextParam()}, {},
                      LanguageTarget(kJavaLanguageName));
}

void PrefixCodeAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                 ApplyContext& context) const {
  const void* node = declaration.member != nullptr ? static_cast<const void*>(declaration.member)
                                                   : static_cast<const void*>(declaration.klass);
  context.effects.Table<JavaModifiers>().Ensure(node).prefix_code =
      annotation.TextArgument(kArgumentParameter);
}

AnnotationMetadata SuffixCodeAnnotation::CreateMetadata() {
  return MakeMetadata("后缀代码", kAnyMember, Phases(AnnotationPhase::kApply), {TextParam()}, {},
                      LanguageTarget(kJavaLanguageName));
}

void SuffixCodeAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                 ApplyContext& context) const {
  const void* node = declaration.member != nullptr ? static_cast<const void*>(declaration.member)
                                                   : static_cast<const void*>(declaration.klass);
  context.effects.Table<JavaModifiers>().Ensure(node).suffix_code =
      annotation.TextArgument(kArgumentParameter);
}

AnnotationMetadata TypeAliasAnnotation::CreateMetadata() {
  return DeclareMetadata("指代类", kLevelClass, {TextParam()});
}

void TypeAliasAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                  DeclareContext& context) const {
  context.semantics.ClassFactsFor(declaration.klass).external_types["java"] =
      annotation.TextArgument(kArgumentParameter);
}

AnnotationMetadata ImportJavaAnnotation::CreateMetadata() {
  return MakeMetadata("导入Java", kAnyMember, Phases(AnnotationPhase::kApply), {TextParam()}, {},
                      LanguageTarget(kJavaLanguageName));
}

void ImportJavaAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                 ApplyContext& context) const {
  const std::string path = annotation.TextArgument(kArgumentParameter);
  if (path.empty())
    return;
  const void* node = declaration.member != nullptr ? static_cast<const void*>(declaration.member)
                                                   : static_cast<const void*>(declaration.klass);
  if (node == nullptr)
    return;
  JavaImportList& list = context.effects.Table<JavaImportList>().Ensure(node);
  if (std::find(list.imports.begin(), list.imports.end(), path) == list.imports.end())
    list.imports.push_back(path);
}

AnnotationMetadata AsyncMethodAnnotation::CreateMetadata() {
  return DeclareMetadata("异步方法", kLevelMethod, {}, {{"独立线程", ParamKind::kBool}});
}

void AsyncMethodAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                    DeclareContext& context) const {
  MemberFacts& facts = context.semantics.MemberFactsFor(declaration.member);
  facts.async = true;
  const std::string independent = annotation.StringArgument("独立线程");
  facts.async_independent_thread = !independent.empty() && IsTruthyLiteral(independent);
}

AnnotationMetadata InlineCodeAnnotation::CreateMetadata() {
  return MakeMetadata("嵌入式代码", kLevelMethod, Phases(AnnotationPhase::kApply), {},
                      {{"语句块标记", ParamKind::kText},
                       {"下一条语句", ParamKind::kText},
                       {"语句块类型", ParamKind::kText}},
                      LanguageTarget(kJavaLanguageName));
}

void InlineCodeAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                 ApplyContext& context) const {
  if (declaration.member == nullptr)
    return;
  JavaInlineCodeSpec& spec = context.effects.Table<JavaInlineCodeSpec>().Ensure(declaration.member);
  spec.statement_marker = annotation.TextArgument("语句块标记");
  spec.next_statement = annotation.TextArgument("下一条语句");
  spec.block_type = annotation.TextArgument("语句块类型");
}

AnnotationMetadata OperatorOverloadAnnotation::CreateMetadata() {
  return MakeMetadata("运算符重载", kLevelMethod, 0, {}, {}, LanguageTarget(kJavaLanguageName));
}

AnnotationMetadata ImplicitOperationAnnotation::CreateMetadata() {
  return MakeMetadata("隐式操作", kLevelMethod, Phases(AnnotationPhase::kApply), {TextParam()}, {},
                      LanguageTarget(kJavaLanguageName));
}

void ImplicitOperationAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                        ApplyContext& context) const {
  if (declaration.member == nullptr)
    return;
  context.effects.Table<JavaImplicitOperationSpec>().Ensure(declaration.member).kind =
      annotation.TextArgument(kArgumentParameter);
}

AnnotationMetadata PropertyDemandAnnotation::CreateMetadata() {
  return DeclareMetadata("属性需求", kLevelMethod, {AnyParam()});
}

void PropertyDemandAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                       DeclareContext& context) const {
  context.semantics.MemberFactsFor(declaration.member).property_demand =
      annotation.TextArgument(kArgumentParameter);
}

AnnotationMetadata DebugAnnotation::CreateMetadata() {
  return MakeMetadata("调试", kLevelMethod, Phases(AnnotationPhase::kApply), {}, {},
                      LanguageTarget(kJavaLanguageName));
}

void DebugAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                            ApplyContext& context) const {
  (void)annotation;
  if (declaration.member != nullptr)
    context.effects.Table<JavaDebugSpec>().Ensure(declaration.member).enabled = true;
}

void RegisterJavaAnnotations(ExtensionRegistry& registry) {
  registry.Register(std::make_unique<OutputNameAnnotation>());
  registry.Register(std::make_unique<HiddenAnnotation>());
  registry.Register(std::make_unique<StaticAnnotation>());
  registry.Register(std::make_unique<PrefixCodeAnnotation>());
  registry.Register(std::make_unique<SuffixCodeAnnotation>());
  registry.Register(std::make_unique<TypeAliasAnnotation>());
  registry.Register(std::make_unique<ImportJavaAnnotation>());
  registry.Register(std::make_unique<AsyncMethodAnnotation>());
  registry.Register(std::make_unique<InlineCodeAnnotation>());
  registry.Register(std::make_unique<OperatorOverloadAnnotation>());
  registry.Register(std::make_unique<ImplicitOperationAnnotation>());
  registry.Register(std::make_unique<PropertyDemandAnnotation>());
  registry.Register(std::make_unique<DebugAnnotation>());
}

}
