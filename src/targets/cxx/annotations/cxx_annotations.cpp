#include "cxx_annotations.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "annotation_context.h"
#include "constants.h"
#include "cxx_effect_types.h"
#include "cxx_language_constants.h"

namespace tiecodec {
namespace {

bool IsValidCxxIdentifier(const std::string& name) {
  if (name.empty())
    return false;
  bool first = true;
  for (char raw : name) {
    const unsigned char c = static_cast<unsigned char>(raw);
    const bool ascii_alpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    const bool ascii_digit = c >= '0' && c <= '9';
    if (first) {
      if (!(ascii_alpha || c == '_' || c >= 0x80))
        return false;
      first = false;
      continue;
    }
    if (!(ascii_alpha || ascii_digit || c == '_' || c >= 0x80))
      return false;
  }
  return true;
}

AnnotationMetadata CxxMetadata(std::string name, unsigned levels, unsigned phases,
                               std::vector<ParamRule> positional = {},
                               std::vector<ParamRule> named = {},
                               FileSearchKind file_kind = FileSearchKind::kNone) {
  return MakeMetadata(std::move(name), levels, phases, std::move(positional), std::move(named),
                      LanguageTarget(kCxxLanguageName), 0, file_kind);
}

const void* NodeOf(const DeclRef& declaration) {
  if (declaration.member != nullptr)
    return static_cast<const void*>(declaration.member);
  return static_cast<const void*>(declaration.klass);
}

}

AnnotationMetadata CxxTypeAliasAnnotation::CreateMetadata() {
  return CxxMetadata("指代类", kLevelClass, Phases(AnnotationPhase::kDeclare), {TextParam()});
}

void CxxTypeAliasAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                     DeclareContext& context) const {
  if (declaration.klass == nullptr)
    return;
  context.semantics.ClassFactsFor(declaration.klass).external_types[kCxxTypeAliasKey] =
      annotation.TextArgument(kArgumentParameter);
}

AnnotationMetadata CxxHiddenAnnotation::CreateMetadata() {
  return CxxMetadata("隐藏", kAnyMember,
                     Phases(AnnotationPhase::kDeclare) | Phases(AnnotationPhase::kApply));
}

void CxxHiddenAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                  DeclareContext& context) const {
  (void)annotation;
  if (declaration.member != nullptr)
    context.semantics.MemberFactsFor(declaration.member).hidden = true;
}

void CxxHiddenAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                ApplyContext& context) const {
  (void)annotation;
  const void* node = NodeOf(declaration);
  if (node == nullptr)
    return;
  context.effects.Table<CxxModifiers>().Ensure(node).hidden = true;
}

AnnotationMetadata CxxStaticAnnotation::CreateMetadata() {
  return CxxMetadata("静态", kLevelVariable | kLevelMethod, Phases(AnnotationPhase::kApply));
}

void CxxStaticAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                ApplyContext& context) const {
  (void)annotation;
  if (declaration.member == nullptr)
    return;
  context.effects.Table<CxxModifiers>().Ensure(declaration.member).is_static = true;
}

AnnotationMetadata CxxInlineCodeAnnotation::CreateMetadata() {
  return CxxMetadata("嵌入式代码", kLevelMethod | kLevelVariable, Phases(AnnotationPhase::kApply));
}

void CxxInlineCodeAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                    ApplyContext& context) const {
  (void)annotation;
  if (declaration.member == nullptr)
    return;
  context.effects.Table<CxxModifiers>().Ensure(declaration.member).inline_code = true;
}

AnnotationMetadata CxxOperatorOverloadAnnotation::CreateMetadata() {
  return CxxMetadata("运算符重载", kLevelMethod, 0);
}

AnnotationMetadata CxxExternalDependencyAnnotation::CreateMetadata() {
  return CxxMetadata("外部依赖库", kAnyMember, Phases(AnnotationPhase::kApply), {TextParam()}, {},
                     FileSearchKind::kLibrary);
}

void CxxExternalDependencyAnnotation::Apply(const Annotation& annotation,
                                            const DeclRef& declaration,
                                            ApplyContext& context) const {
  const std::string path = annotation.TextArgument(kArgumentParameter);
  if (path.empty())
    return;
  CxxExternalDependency& list = context.effects.Table<CxxExternalDependency>().Ensure(
      declaration.member != nullptr ? static_cast<const void*>(declaration.member)
                                    : static_cast<const void*>(declaration.klass));
  if (std::find(list.libraries.begin(), list.libraries.end(), path) == list.libraries.end())
    list.libraries.push_back(path);
}

AnnotationMetadata CxxAsyncMethodAnnotation::CreateMetadata() {
  return CxxMetadata("异步方法", kLevelMethod, Phases(AnnotationPhase::kDeclare), {},
                     {{"独立线程", ParamKind::kBool}});
}

void CxxAsyncMethodAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                       DeclareContext& context) const {
  (void)annotation;
  if (declaration.member == nullptr)
    return;
  MemberFacts& facts = context.semantics.MemberFactsFor(declaration.member);
  facts.async = true;
  const AnnotationArgument* independent = annotation.FindArgument("独立线程");
  if (independent != nullptr) {
    const std::string value = annotation.UnwrappedArgument(*independent);
    facts.async_independent_thread = value == "真" || value == "true";
  }
}

AnnotationMetadata CxxOutputNameAnnotation::CreateMetadata() {
  return CxxMetadata("输出名", kAllLevels, Phases(AnnotationPhase::kDeclare), {TextParam()});
}

void CxxOutputNameAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                      DeclareContext& context) const {
  const std::string value = annotation.TextArgument(kArgumentParameter);
  if (declaration.level == AnnotationLevel::kUnit) {
    if (declaration.unit != nullptr && !value.empty())
      context.semantics.UnitFactsFor(declaration.unit).output_package = value;
    return;
  }
  if (declaration.level == AnnotationLevel::kClass) {
    if (declaration.klass == nullptr)
      return;
    if (!IsValidCxxIdentifier(value)) {
      context.diagnostics.Error(context.unit.file, declaration.klass->position,
                                "类输出名不是合法的 C++ 标识符: " + value, "enter.output-name");
      return;
    }
    context.semantics.ClassFactsFor(declaration.klass).output_name = value;
    return;
  }
  if (declaration.member == nullptr)
    return;
  if (!IsValidCxxIdentifier(value)) {
    context.diagnostics.Error(context.unit.file, declaration.member->position,
                              "成员输出名不是合法的 C++ 标识符: " + value, "enter.output-name");
    return;
  }
  context.semantics.MemberFactsFor(declaration.member).output_name = value;
}

AnnotationMetadata CxxExternalSourceAnnotation::CreateMetadata() {
  return CxxMetadata("外部源文件", kLevelClass, Phases(AnnotationPhase::kApply), {TextParam()}, {},
                     FileSearchKind::kSource);
}

void CxxExternalSourceAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                        ApplyContext& context) const {
  const std::string path = annotation.TextArgument(kArgumentParameter);
  if (path.empty() || declaration.klass == nullptr)
    return;
  CxxExternalSource& list = context.effects.Table<CxxExternalSource>().Ensure(declaration.klass);
  if (std::find(list.files.begin(), list.files.end(), path) == list.files.end())
    list.files.push_back(path);
}

AnnotationMetadata CxxIncludeAnnotation::CreateMetadata() {
  return CxxMetadata("导入头文件", kAnyMember, Phases(AnnotationPhase::kApply), {TextParam()});
}

void CxxIncludeAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                 ApplyContext& context) const {
  const std::string path = annotation.TextArgument(kArgumentParameter);
  if (path.empty())
    return;
  const void* node = NodeOf(declaration);
  if (node == nullptr)
    return;
  CxxIncludeList& list = context.effects.Table<CxxIncludeList>().Ensure(node);
  if (std::find(list.includes.begin(), list.includes.end(), path) == list.includes.end())
    list.includes.push_back(path);
}

void RegisterCxxAnnotations(ExtensionRegistry& registry) {
  registry.Register(std::make_unique<CxxTypeAliasAnnotation>());
  registry.Register(std::make_unique<CxxHiddenAnnotation>());
  registry.Register(std::make_unique<CxxStaticAnnotation>());
  registry.Register(std::make_unique<CxxInlineCodeAnnotation>());
  registry.Register(std::make_unique<CxxOperatorOverloadAnnotation>());
  registry.Register(std::make_unique<CxxOutputNameAnnotation>());
  registry.Register(std::make_unique<CxxExternalSourceAnnotation>());
  registry.Register(std::make_unique<CxxExternalDependencyAnnotation>());
  registry.Register(std::make_unique<CxxIncludeAnnotation>());
  registry.Register(std::make_unique<CxxAsyncMethodAnnotation>());
}

}
