#include "android_annotations.h"

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "android_constants.h"
#include "android_effect_types.h"
#include "annotation_context.h"
#include "constants.h"
#include "extension_registry.h"
#include "symbol_resolution.h"

namespace tiecodec {
namespace {

AnnotationMetadata AndroidMetadata(std::string name, unsigned levels, unsigned phases,
                                   std::vector<ParamRule> positional = {},
                                   std::vector<ParamRule> named = {},
                                   FileSearchKind file_kind = FileSearchKind::kNone) {
  return MakeMetadata(std::move(name), levels, phases, std::move(positional), std::move(named),
                      PlatformTarget(kAndroidPlatformName), 0, file_kind);
}

std::string StripGenerics(const std::string& name) {
  const std::size_t open = name.find('<');
  if (open == std::string::npos)
    return name;
  const std::size_t close = name.rfind('>');
  if (close == std::string::npos || close < open)
    return name;
  return name.substr(0, open) + name.substr(close + 1);
}

std::string SimpleTypeName(const std::string& type_name) {
  const std::string base = StripGenerics(type_name);
  const std::size_t dot = base.rfind('.');
  return dot == std::string::npos ? base : base.substr(dot + 1);
}

std::string TrimText(const std::string& text) {
  std::size_t begin = 0;
  std::size_t end = text.size();
  while (begin < end &&
         (text[begin] == ' ' || text[begin] == '\t' || text[begin] == '\n' || text[begin] == '\r'))
    ++begin;
  while (end > begin && (text[end - 1] == ' ' || text[end - 1] == '\t' || text[end - 1] == '\n' ||
                         text[end - 1] == '\r'))
    --end;
  return text.substr(begin, end - begin);
}

std::vector<std::string> TlyAttributeKeys(const std::string& text) {
  std::vector<std::string> keys;
  const std::size_t open = text.find('{');
  if (open == std::string::npos)
    return keys;
  const auto add_key = [&keys](const std::string& raw) {
    const std::string trimmed = TrimText(raw);
    if (trimmed.empty())
      return;
    const std::size_t equal = trimmed.find('=');
    const std::string name =
        TrimText(equal == std::string::npos ? trimmed : trimmed.substr(0, equal));
    if (!name.empty())
      keys.push_back(name);
  };
  std::string current;
  int depth = 0;
  for (std::size_t i = open + 1; i < text.size(); ++i) {
    const char c = text[i];
    if (c == '{') {
      ++depth;
    } else if (c == '}') {
      if (depth == 0) {
        add_key(current);
        return keys;
      }
      --depth;
    } else if (c == ',' && depth == 0) {
      add_key(current);
      current.clear();
      continue;
    }
    if (depth > 0 || c != '}')
      current.push_back(c);
  }
  add_key(current);
  return keys;
}

const AnnotationArgument* FindTlyArgument(const Annotation& annotation, bool allow_text) {
  for (const AnnotationArgument& argument : annotation.arguments) {
    if (argument.value.find('{') != std::string::npos)
      return &argument;
    if (allow_text && argument.is_string)
      return &argument;
  }
  return nullptr;
}

const AnnotationArgument* FindNamedArgument(const Annotation& annotation, const std::string& name) {
  for (const AnnotationArgument& argument : annotation.arguments) {
    if (argument.name == name)
      return &argument;
  }
  return nullptr;
}

const AnnotationArgument* FindTlyTableArgument(const Annotation& annotation) {
  for (const AnnotationArgument& argument : annotation.arguments) {
    if (argument.name.empty() || argument.name == kArgumentParameter)
      return &argument;
  }
  return nullptr;
}

const AnnotationArgument* FindValueArgument(const Annotation& annotation) {
  for (const AnnotationArgument& argument : annotation.arguments) {
    if (argument.name.empty() || argument.name == kArgumentParameter)
      return &argument;
  }
  return nullptr;
}

const void* EffectNode(const DeclRef& declaration) {
  return declaration.member != nullptr ? static_cast<const void*>(declaration.member)
                                       : static_cast<const void*>(declaration.klass);
}

void AppendResourceCopy(ApplyContext& context, const DeclRef& declaration,
                        AndroidResourceCopySpec::Kind kind, const std::string& path) {
  if (path.empty())
    return;
  context.effects.Table<AndroidResourceCopies>()
      .Ensure(EffectNode(declaration))
      .entries.push_back(AndroidResourceCopySpec{kind, path});
}

}

void ExtraPermissionAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                      ApplyContext& context) const {
  const AnnotationArgument* value = FindValueArgument(annotation);
  if (value == nullptr || value->value.empty())
    return;
  context.effects.Table<AndroidManifestSpecs>()
      .Ensure(EffectNode(declaration))
      .entries.push_back(AndroidManifestEntrySpec{"permission", value->value});
}

void ExtraManifestAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                    ApplyContext& context) const {
  const AnnotationArgument* value = FindValueArgument(annotation);
  if (value == nullptr || value->value.empty())
    return;
  const AnnotationArgument* node = FindNamedArgument(annotation, "节点");
  context.effects.Table<AndroidManifestSpecs>()
      .Ensure(EffectNode(declaration))
      .entries.push_back(
          AndroidManifestEntrySpec{node == nullptr ? std::string() : node->value, value->value});
}

void ExtraResourceAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                    ApplyContext& context) const {
  AppendResourceCopy(context, declaration, AndroidResourceCopySpec::Kind::kExtraResource,
                     annotation.TextArgument(kArgumentParameter));
}

void AndroidResourceAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                      ApplyContext& context) const {
  AndroidResourceCopySpec::Kind kind = AndroidResourceCopySpec::Kind::kAndroidResource;
  if (metadata_.name == "安卓资源.图片")
    kind = AndroidResourceCopySpec::Kind::kImage;
  else if (metadata_.name == "安卓资源.动画")
    kind = AndroidResourceCopySpec::Kind::kAnimation;
  else if (metadata_.name == "安卓资源.布局")
    kind = AndroidResourceCopySpec::Kind::kLayout;
  else if (metadata_.name == "安卓资源.XML")
    kind = AndroidResourceCopySpec::Kind::kXml;
  AppendResourceCopy(context, declaration, kind, annotation.TextArgument(kArgumentParameter));
}

void ExternalJavaFileAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                       ApplyContext& context) const {
  AppendResourceCopy(context, declaration, AndroidResourceCopySpec::Kind::kExternalJavaFile,
                     annotation.TextArgument(kArgumentParameter));
}

void ExternalLibraryAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                      ApplyContext& context) const {
  const std::string path = annotation.TextArgument(kArgumentParameter);
  if (path.empty())
    return;
  bool compile_only = false;
  if (const AnnotationArgument* flag = FindNamedArgument(annotation, "不打包"))
    compile_only = flag->value == "真";
  context.effects.Table<AndroidLibraries>()
      .Ensure(EffectNode(declaration))
      .entries.push_back(AndroidLibrarySpec{path, compile_only});
}

void ExternalNativeLibraryAnnotation::Apply(const Annotation& annotation,
                                            const DeclRef& declaration,
                                            ApplyContext& context) const {
  AppendResourceCopy(context, declaration, AndroidResourceCopySpec::Kind::kNativeLibrary,
                     annotation.TextArgument(kArgumentParameter));
}

AnnotationMetadata ExtraPermissionAnnotation::CreateMetadata() {
  return AndroidMetadata("附加权限", kAnyMember, Phases(AnnotationPhase::kApply), {TextParam()});
}

AnnotationMetadata ExtraManifestAnnotation::CreateMetadata() {
  return AndroidMetadata("附加清单", kAnyMember, Phases(AnnotationPhase::kApply), {TextParam()},
                         {{"节点", ParamKind::kText}});
}

AnnotationMetadata ExtraResourceAnnotation::CreateMetadata() {
  return AndroidMetadata("附加资源", kAnyMember, Phases(AnnotationPhase::kApply), {TextParam()});
}

AndroidResourceAnnotation::AndroidResourceAnnotation(std::string name) {
  metadata_ = AndroidMetadata(std::move(name), kAnyMember, Phases(AnnotationPhase::kApply),
                              {TextParam()}, {}, FileSearchKind::kResource);
}

AnnotationMetadata AndroidResourceAnnotation::Metadata() const {
  return metadata_;
}

AnnotationMetadata ExternalJavaFileAnnotation::CreateMetadata() {
  return AndroidMetadata("外部Java文件", kAnyMember, Phases(AnnotationPhase::kApply), {TextParam()},
                         {}, FileSearchKind::kSource);
}

AnnotationMetadata ExternalLibraryAnnotation::CreateMetadata() {
  return AndroidMetadata("外部依赖库", kAnyMember, Phases(AnnotationPhase::kApply), {TextParam()},
                         {{"不打包", ParamKind::kBool}}, FileSearchKind::kLibrary);
}

AnnotationMetadata ExternalNativeLibraryAnnotation::CreateMetadata() {
  return AndroidMetadata("外部动态库", kAnyMember, Phases(AnnotationPhase::kApply), {TextParam()},
                         {}, FileSearchKind::kLibrary);
}

AnnotationMetadata ComponentConfigAnnotation::CreateMetadata() {
  return AndroidMetadata("组件配置", kLevelVariable,
                         AnnotationPhase::kDeclare | AnnotationPhase::kApply,
                         {{kArgumentParameter, ParamKind::kTly}},
                         {{"根布局", ParamKind::kBool}, {"父布局", ParamKind::kMemberName}});
}

void ComponentConfigAnnotation::Validate(const Annotation& annotation, const DeclRef& declaration,
                                         ValidationContext& context) const {
  if (context.type_error || declaration.member == nullptr)
    return;
  const AnnotationArgument* tly = FindTlyArgument(annotation, false);
  if (tly == nullptr)
    return;
  const Member& member = *declaration.member;
  if (!context.index.ClassChainContains(member.type_name, kWindowComponentClass)) {
    context.diagnostics.Error(context.unit.file, annotation.symbol_position,
                              "\"组件配置\" 只能应用在窗口组件类型的变量上",
                              "parse.annotation.component-variable");
    return;
  }
  for (const std::string& key : TlyAttributeKeys(tly->value)) {
    if (!key.empty() && key.front() == '$')
      continue;
    if (context.index.ResolveMemberName(key, StripGenerics(member.type_name)))
      continue;
    context.diagnostics.Error(
        context.unit.file, annotation.symbol_position,
        "类\"" + SimpleTypeName(member.type_name) + "\"中找不到属性\"" + key + "\"",
        "parse.annotation.unknown-attribute");
    break;
  }
}

void ComponentConfigAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                        DeclareContext& context) const {
  (void)annotation;
  context.semantics.MemberFactsFor(declaration.member).force_use = true;
}

void ComponentConfigAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                      ApplyContext& context) const {
  if (declaration.member == nullptr || declaration.owner == nullptr)
    return;
  context.plan.ForceMember(declaration.owner, declaration.member);
  AndroidComponentSpec& spec =
      context.effects.Table<AndroidComponentSpec>().Ensure(declaration.member);
  spec.layout = false;
  if (const AnnotationArgument* parent = FindNamedArgument(annotation, "父布局"))
    spec.parent = parent->value;
  if (const AnnotationArgument* root = FindNamedArgument(annotation, "根布局"))
    spec.root = root->value == "真";
  const AnnotationArgument* table = FindTlyTableArgument(annotation);
  if (table == nullptr)
    return;
  spec.table = table->value;
  const ClassInfo* type_class =
      context.symbols.Resolve(declaration.member->type_name, declaration.owner);
  if (type_class == nullptr)
    return;
  std::string parent_value;
  if (const AnnotationArgument* parent = FindNamedArgument(annotation, "父布局"))
    parent_value = parent->value;
  for (const std::string& key : TlyAttributeKeys(table->value)) {
    if (key.empty() || key == "根布局" || key == "父布局")
      continue;
    if (key.front() == '$') {
      const std::string property_name = TrimText(key.substr(1));
      if (property_name.empty() || parent_value.empty())
        continue;
      const Member* parent_member =
          FindMemberInChain(context.symbols, *declaration.owner, parent_value, nullptr);
      if (parent_member == nullptr)
        continue;
      const ClassInfo* parent_type =
          context.symbols.Resolve(parent_member->type_name, declaration.owner);
      if (parent_type == nullptr)
        continue;
      const ClassInfo* property_owner = nullptr;
      if (const Member* property =
              FindMemberInChain(context.symbols, *parent_type, property_name, &property_owner)) {
        context.plan.ReferenceMember(property_owner, property);
      }
      continue;
    }
    const ClassInfo* property_owner = nullptr;
    if (const Member* property =
            FindMemberInChain(context.symbols, *type_class, key, &property_owner)) {
      context.plan.ReferenceMember(property_owner, property);
    }
  }
}

AnnotationMetadata LayoutConfigAnnotation::CreateMetadata() {
  return AndroidMetadata("布局配置", kLevelVariable,
                         AnnotationPhase::kDeclare | AnnotationPhase::kApply, {TextParam()});
}

void LayoutConfigAnnotation::Validate(const Annotation& annotation, const DeclRef& declaration,
                                      ValidationContext& context) const {
  if (context.type_error || declaration.member == nullptr)
    return;
  if (FindTlyArgument(annotation, true) == nullptr)
    return;
  if (!context.index.ClassChainContains(declaration.member->type_name, kLayoutComponentClass)) {
    context.diagnostics.Error(context.unit.file, annotation.symbol_position,
                              "\"布局配置\" 中父布局必须为布局组件成员变量",
                              "parse.annotation.layout-variable");
  }
}

void LayoutConfigAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                     DeclareContext& context) const {
  (void)annotation;
  context.semantics.MemberFactsFor(declaration.member).force_use = true;
}

void LayoutConfigAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                   ApplyContext& context) const {
  if (declaration.member == nullptr)
    return;
  context.plan.ForceMember(declaration.owner, declaration.member);
  AndroidComponentSpec& spec =
      context.effects.Table<AndroidComponentSpec>().Ensure(declaration.member);
  spec.layout = true;
  if (const AnnotationArgument* table = FindValueArgument(annotation))
    spec.table = table->value;
}

AnnotationMetadata ThemeConfigAnnotation::CreateMetadata() {
  return AndroidMetadata(
      "主题配置", kLevelVariable, AnnotationPhase::kDeclare | AnnotationPhase::kApply,
      {{kArgumentParameter, ParamKind::kTly}}, {{"类型", ParamKind::kText, false, true}});
}

void ThemeConfigAnnotation::Validate(const Annotation& annotation, const DeclRef& declaration,
                                     ValidationContext& context) const {
  if (context.type_error || declaration.member == nullptr)
    return;
  if (FindTlyArgument(annotation, false) == nullptr)
    return;
  if (context.index.QualifiedNameOf(StripGenerics(declaration.member->type_name)) !=
      "结绳.安卓.主题资源") {
    context.diagnostics.Error(context.unit.file, annotation.symbol_position,
                              "\"主题配置\" 只能应用在 \"主题资源\" 类型的变量上",
                              "parse.annotation.theme-variable");
  }
}

void ThemeConfigAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                    DeclareContext& context) const {
  (void)annotation;
  context.semantics.MemberFactsFor(declaration.member).force_use = true;
}

void ThemeConfigAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                  ApplyContext& context) const {
  if (declaration.member == nullptr || declaration.owner == nullptr)
    return;
  context.plan.ForceMember(declaration.owner, declaration.member);
  AndroidThemeConfigSpec& spec =
      context.effects.Table<AndroidThemeConfigSpec>().Ensure(declaration.member);
  if (const AnnotationArgument* table = FindTlyTableArgument(annotation))
    spec.table = table->value;
  if (const AnnotationArgument* parent = FindNamedArgument(annotation, "类型"))
    spec.parent = annotation.UnwrappedArgument(*parent);
  const ClassInfo* type = context.symbols.Resolve(declaration.member->type_name, declaration.owner);
  if (type != nullptr)
    context.plan.ReferenceClass(type);
}

AnnotationMetadata I18nAnnotation::CreateMetadata() {
  return AndroidMetadata("国际化", kLevelVariable, Phases(AnnotationPhase::kApply),
                         {{kArgumentParameter, ParamKind::kAny, true}},
                         {{"地区", ParamKind::kAny}});
}

void I18nAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                           ApplyContext& context) const {
  if (declaration.member == nullptr)
    return;
  AndroidI18nSpec& spec = context.effects.Table<AndroidI18nSpec>().Ensure(declaration.member);
  AndroidI18nEntry entry;
  entry.text = annotation.TextArgument(kArgumentParameter);
  for (const AnnotationArgument& argument : annotation.arguments) {
    if (argument.name == "地区")
      entry.locale = annotation.UnwrappedArgument(argument);
    else if (argument.name.empty())
      entry.text = annotation.UnwrappedArgument(argument);
  }
  spec.entries.push_back(std::move(entry));
}

AnnotationMetadata GlobalThemeAnnotation::CreateMetadata() {
  return AndroidMetadata("全局主题", kLevelClass, Phases(AnnotationPhase::kApply), {TextParam()});
}

void GlobalThemeAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                  ApplyContext& context) const {
  context.effects.Table<AndroidGlobalThemeSpec>().Ensure(declaration.klass).name =
      annotation.TextArgument(kArgumentParameter);
}

AnnotationMetadata WindowThemeAnnotation::CreateMetadata() {
  return AndroidMetadata("窗口主题", kLevelClass, Phases(AnnotationPhase::kApply), {TextParam()});
}

void WindowThemeAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                  ApplyContext& context) const {
  context.effects.Table<AndroidWindowThemeSpec>().Ensure(declaration.klass).name =
      annotation.TextArgument(kArgumentParameter);
}

AnnotationMetadata LayoutPropertyAnnotation::CreateMetadata() {
  return AndroidMetadata("布局属性", kLevelMethod, 0);
}

AnnotationMetadata RequiredValueTypeAnnotation::CreateMetadata() {
  return AndroidMetadata("需求值类型", kLevelClass, Phases(AnnotationPhase::kDeclare),
                         {{kArgumentParameter, ParamKind::kTypeName}});
}

void RequiredValueTypeAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                          DeclareContext& context) const {
  (void)annotation;
  context.semantics.ClassFactsFor(declaration.klass).value_type = true;
}

AnnotationMetadata ConstantTypeAnnotation::CreateMetadata() {
  return AndroidMetadata("常量类型", kLevelClass,
                         Phases(AnnotationPhase::kDeclare) | Phases(AnnotationPhase::kApply),
                         {{kArgumentParameter, ParamKind::kTypeName}});
}

void ConstantTypeAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                     DeclareContext& context) const {
  (void)annotation;
  context.semantics.ClassFactsFor(declaration.klass).value_type = true;
}

void ConstantTypeAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                   ApplyContext& context) const {
  context.effects.Table<AndroidConstantTypeSpec>().Ensure(declaration.klass).type =
      annotation.TextArgument(kArgumentParameter);
}

AnnotationMetadata ValueOutputRuleAnnotation::CreateMetadata() {
  return AndroidMetadata("值输出规则", kLevelClass,
                         Phases(AnnotationPhase::kDeclare) | Phases(AnnotationPhase::kApply),
                         {TextParam()});
}

void ValueOutputRuleAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                        DeclareContext& context) const {
  context.semantics.ClassFactsFor(declaration.klass).value_output_rule =
      annotation.TextArgument(kArgumentParameter);
}

void ValueOutputRuleAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                      ApplyContext& context) const {
  context.effects.Table<AndroidValueOutputRuleSpec>().Ensure(declaration.klass).rule =
      annotation.TextArgument(kArgumentParameter);
}

AnnotationMetadata MatchFileAnnotation::CreateMetadata() {
  return AndroidMetadata("匹配文件", kLevelClass, Phases(AnnotationPhase::kDeclare), {TextParam()});
}

void MatchFileAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                  DeclareContext& context) const {
  context.semantics.ClassFactsFor(declaration.klass).match_file_pattern =
      annotation.TextArgument(kArgumentParameter);
}

AnnotationMetadata GlobalApplicationAnnotation::CreateMetadata() {
  return AndroidMetadata("全局应用", kLevelClass,
                         AnnotationPhase::kDeclare | AnnotationPhase::kApply);
}

void GlobalApplicationAnnotation::Declare(const Annotation& annotation, const DeclRef& declaration,
                                          DeclareContext& context) const {
  (void)annotation;
  context.semantics.ClassFactsFor(declaration.klass).force_output = true;
}

void GlobalApplicationAnnotation::Apply(const Annotation& annotation, const DeclRef& declaration,
                                        ApplyContext& context) const {
  (void)annotation;
  context.plan.ForceClass(declaration.owner);
  context.effects.Table<AndroidApplicationSpec>().Ensure(declaration.klass);
}

void RegisterAndroidAnnotations(ExtensionRegistry& registry) {
  registry.Register(std::make_unique<ExtraPermissionAnnotation>());
  registry.Register(std::make_unique<ExtraManifestAnnotation>());
  registry.Register(std::make_unique<ExtraResourceAnnotation>());
  registry.Register(std::make_unique<AndroidResourceAnnotation>("安卓资源"));
  registry.Register(std::make_unique<AndroidResourceAnnotation>("安卓资源.图片"));
  registry.Register(std::make_unique<AndroidResourceAnnotation>("安卓资源.动画"));
  registry.Register(std::make_unique<AndroidResourceAnnotation>("安卓资源.布局"));
  registry.Register(std::make_unique<AndroidResourceAnnotation>("安卓资源.XML"));
  registry.Register(std::make_unique<ExternalJavaFileAnnotation>());
  registry.Register(std::make_unique<ExternalLibraryAnnotation>());
  registry.Register(std::make_unique<ExternalNativeLibraryAnnotation>());
  registry.Register(std::make_unique<ComponentConfigAnnotation>());
  registry.Register(std::make_unique<LayoutConfigAnnotation>());
  registry.Register(std::make_unique<ThemeConfigAnnotation>());
  registry.Register(std::make_unique<I18nAnnotation>());
  registry.Register(std::make_unique<GlobalThemeAnnotation>());
  registry.Register(std::make_unique<WindowThemeAnnotation>());
  registry.Register(std::make_unique<LayoutPropertyAnnotation>());
  registry.Register(std::make_unique<RequiredValueTypeAnnotation>());
  registry.Register(std::make_unique<ConstantTypeAnnotation>());
  registry.Register(std::make_unique<ValueOutputRuleAnnotation>());
  registry.Register(std::make_unique<MatchFileAnnotation>());
  registry.Register(std::make_unique<GlobalApplicationAnnotation>());
}

}
