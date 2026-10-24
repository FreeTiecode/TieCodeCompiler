#include "annotation_rules.h"

#include "constants.h"

#include <cctype>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "compile_conditions.h"
#include "file_search.h"

namespace tiecodec {
namespace {

enum Level {
  kLevelUnit = 1,
  kLevelClass = 2,
  kLevelVariable = 4,
  kLevelMethod = 8,
};

enum class ParamKind {
  kAny,
  kText,
  kBool,
  kTypeName,
  kMemberName,
  kCondition,
  kTly,
};

struct ParamRule {
  std::string name;
  ParamKind kind = ParamKind::kAny;
  bool optional = false;
  bool required = false;
};

struct AnnotationRule {
  int levels = 0;
  std::vector<ParamRule> positional;
  std::vector<ParamRule> named;
};

const std::map<std::string, AnnotationRule>& AnnotationRules() {
  static const std::map<std::string, AnnotationRule> rules = [] {
    std::map<std::string, AnnotationRule> result;
    const int any_member = kLevelClass | kLevelVariable | kLevelMethod;
    const int all_levels = kLevelUnit | any_member;
    const auto add = [&result](const std::string& name, int levels,
                               const std::vector<ParamRule>& positional = {},
                               const std::vector<ParamRule>& named = {}) {
      AnnotationRule rule;
      rule.levels = levels;
      rule.positional = positional;
      rule.named = named;
      result[name] = rule;
    };
    const ParamRule kTextParam{kArgumentParameter, ParamKind::kText};
    add(annotation::kOutputName, all_levels, {kTextParam});
    add(annotation::kHidden, any_member);
    add("废弃使用", any_member, {kTextParam});
    add(annotation::kForcedOutput, any_member, {}, {{kArgumentParameter, ParamKind::kBool, true}});
    add("编译条件", any_member, {{kArgumentParameter, ParamKind::kCondition}});
    add("附加权限", any_member, {kTextParam});
    add(annotation::kExtraManifest, any_member, {kTextParam},
        {{annotation::kManifestNode, ParamKind::kText}});
    add(annotation::kExtraResource, any_member, {kTextParam});
    add(annotation::kAndroidResource, any_member, {kTextParam});
    add("安卓资源.图片", any_member, {kTextParam});
    add("安卓资源.动画", any_member, {kTextParam});
    add("安卓资源.布局", any_member, {kTextParam});
    add("安卓资源.XML", any_member, {kTextParam});
    add("导入Java", any_member, {kTextParam});
    add(annotation::kExternalJavaFile, any_member, {kTextParam});
    add("外部依赖库", any_member, {kTextParam}, {{"不打包", ParamKind::kBool}});
    add("外部动态库", any_member, {kTextParam});
    add(annotation::kPrefixCode, any_member, {kTextParam});
    add("后缀代码", any_member, {kTextParam});
    add(annotation::kStatic, kLevelVariable | kLevelMethod);
    add("调试", kLevelMethod);
    add("虚拟方法", kLevelMethod);
    add("虚拟事件", kLevelMethod);
    add(annotation::kPropertyRead, kLevelMethod);
    add(annotation::kPropertyWrite, kLevelMethod);
    add("运算符重载", kLevelMethod);
    add("隐式操作", kLevelMethod, {kTextParam});
    add(annotation::kAsyncMethod, kLevelMethod, {}, {{"独立线程", ParamKind::kBool}});
    add("嵌入式代码", kLevelMethod, {},
        {{"语句块标记", ParamKind::kText},
         {"下一条语句", ParamKind::kText},
         {"语句块类型", ParamKind::kText}});
    add("属性需求", kLevelMethod, {{kArgumentParameter, ParamKind::kAny}});
    add("布局属性", kLevelMethod);
    add(annotation::kComponentConfig, kLevelVariable, {{kArgumentParameter, ParamKind::kTly}},
        {{annotation::kRootLayout, ParamKind::kBool},
         {annotation::kParentLayout, ParamKind::kMemberName}});
    add(annotation::kLayoutConfig, kLevelVariable, {kTextParam});
    add(annotation::kThemeConfig, kLevelVariable, {{kArgumentParameter, ParamKind::kTly}},
        {{annotation::kThemeParent, ParamKind::kText, false, true}});
    add(annotation::kI18n, kLevelVariable, {{kArgumentParameter, ParamKind::kAny, true}},
        {{annotation::kI18nLocale, ParamKind::kAny}});
    add("指代类", kLevelClass, {kTextParam});
    add("全局基础类", kLevelClass);
    add("全局应用", kLevelClass);
    add(annotation::kGlobalTheme, kLevelClass, {kTextParam});
    add("窗口主题", kLevelClass, {kTextParam});
    add("全局类", kLevelClass);
    add(annotation::kNoInstantiation, kLevelClass);
    add("禁止继承", kLevelClass);
    add("禁止输出", kLevelClass);
    add("禁用指针", kLevelClass);
    add("外部基础类", kLevelClass, {kTextParam});
    add(annotation::kRequiredValueType, kLevelClass, {{kArgumentParameter, ParamKind::kTypeName}});
    add(annotation::kConstantType, kLevelClass, {{kArgumentParameter, ParamKind::kTypeName}});
    add(annotation::kValueOutputRule, kLevelClass, {kTextParam});
    add(annotation::kMatchFile, kLevelClass, {kTextParam});
    return result;
  }();
  return rules;
}

const char* LevelName(int level) {
  switch (level) {
    case kLevelUnit:
      return "包名";
    case kLevelClass:
      return "类";
    case kLevelVariable:
      return "变量";
    case kLevelMethod:
      return "方法";
    default:
      return "";
  }
}

std::string Unquote(const std::string& value) {
  if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                            (value.front() == '\'' && value.back() == '\''))) {
    return value.substr(1, value.size() - 2);
  }
  return value;
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

bool IsIdentifierLike(const std::string& value) {
  if (value.empty())
    return false;
  for (std::size_t i = 0; i < value.size(); ++i) {
    const unsigned char c = static_cast<unsigned char>(value[i]);
    const bool identifier_byte = c == '_' || c == '.' || c >= 0x80 || (c >= '0' && c <= '9') ||
                                 (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    if (!identifier_byte)
      return false;
  }
  const unsigned char first = static_cast<unsigned char>(value.front());
  return !std::isdigit(first);
}

bool IsNumberLike(const std::string& value) {
  if (value.empty())
    return false;
  const unsigned char first = static_cast<unsigned char>(value.front());
  return std::isdigit(first) != 0 || (first == '-' && value.size() > 1);
}

}

void BuildAnnotationSymbolIndex(const std::vector<Unit>& units, AnnotationSymbolIndex* index) {
  if (index == nullptr)
    return;
  index->class_names.clear();
  index->constant_names.clear();
  index->member_names.clear();
  index->class_constants.clear();
  index->class_members.clear();
  index->forbidden_creation.clear();
  index->builtin_types = {"对象",       "整数",     "文本",       "长整数", "逻辑型",
                          "字符",       "字节",     "单精度小数", "小数",   "遍历器",
                          "键值表条目", "任意类型", "任何类型",   "类型",   "变体型"};
  for (const Unit& unit : units) {
    for (const ClassDecl& class_decl : unit.classes) {
      index->class_names.insert(class_decl.name);
      index->class_base[class_decl.name] = class_decl.base_name;
      index->class_package[class_decl.name] = unit.package_name;
      if (class_decl.HasAnnotation("禁止创建对象"))
        index->forbidden_creation.insert(class_decl.name);
      for (const Member& member : class_decl.members) {
        index->member_names.insert(member.name);
        index->class_members[class_decl.name].insert(member.name);
        if (member.kind != MemberKind::kConstant)
          continue;
        index->constant_names.insert(member.name);
        index->class_constants[class_decl.name].insert(member.name);
      }
    }
  }
}

bool AnnotationSymbolIndex::ResolveSymbol(const std::string& raw_name) const {
  const std::string name = StripGenerics(raw_name);
  if (name.empty())
    return false;
  if (class_names.count(name) != 0 || builtin_types.count(name) != 0 ||
      constant_names.count(name) != 0) {
    return true;
  }
  const std::size_t dot = name.find('.');
  if (dot == std::string::npos)
    return false;
  const std::string class_name = name.substr(0, dot);
  const auto found = class_constants.find(class_name);
  if (found == class_constants.end())
    return false;
  const std::string rest = name.substr(dot + 1);
  if (found->second.count(rest) != 0)
    return true;
  const std::size_t last_dot = rest.rfind('.');
  if (last_dot != std::string::npos && found->second.count(rest.substr(last_dot + 1)) != 0)
    return true;
  return false;
}

bool AnnotationSymbolIndex::ResolveTypeName(const std::string& raw_name) const {
  const std::string name = StripGenerics(raw_name);
  if (name.empty())
    return false;
  return class_names.count(name) != 0 || builtin_types.count(name) != 0;
}

bool AnnotationSymbolIndex::ResolveMemberName(const std::string& raw_name,
                                              const std::string& owner_class) const {
  const std::string name = StripGenerics(raw_name);
  if (name.empty())
    return false;
  const std::size_t dot = name.find('.');
  if (dot != std::string::npos) {
    const std::string class_name = name.substr(0, dot);
    const auto found = class_members.find(class_name);
    if (found != class_members.end()) {
      const std::string rest = name.substr(dot + 1);
      if (found->second.count(rest) != 0)
        return true;
      const std::size_t last_dot = rest.rfind('.');
      if (last_dot != std::string::npos && found->second.count(rest.substr(last_dot + 1)) != 0)
        return true;
    }
    return false;
  }
  if (!owner_class.empty()) {
    std::string current = owner_class;
    std::set<std::string> visited;
    while (!current.empty() && visited.insert(current).second) {
      const auto found = class_members.find(current);
      if (found != class_members.end() && found->second.count(name) != 0)
        return true;
      const auto base = class_base.find(current);
      if (base == class_base.end() || base->second.empty())
        break;
      current = StripGenerics(base->second);
    }
    return false;
  }
  if (member_names.count(name) != 0 || class_names.count(name) != 0)
    return true;
  return false;
}

bool AnnotationSymbolIndex::ClassChainContains(const std::string& type_name,
                                               const std::string& target) const {
  std::string current = StripGenerics(type_name);
  std::set<std::string> visited;
  while (!current.empty() && visited.insert(current).second) {
    if (current == target)
      return true;
    const auto found = class_base.find(current);
    if (found == class_base.end() || found->second.empty())
      break;
    current = StripGenerics(found->second);
  }
  return false;
}

std::string AnnotationSymbolIndex::QualifiedNameOf(const std::string& class_name) const {
  const auto found = class_package.find(class_name);
  if (found == class_package.end() || found->second.empty())
    return class_name;
  return found->second + "." + class_name;
}

namespace {

const ParamRule* FindParam(const std::vector<ParamRule>& params, const std::string& name) {
  for (const ParamRule& param : params) {
    if (param.name == name)
      return &param;
  }
  return nullptr;
}

enum class ValueForm { kString, kBool, kNumber, kIdentifier, kOther };

ValueForm ClassifyValue(const AnnotationArgument& argument) {
  if (argument.is_string)
    return ValueForm::kString;
  const std::string& value = argument.value;
  if (value == "真" || value == "假")
    return ValueForm::kBool;
  if (IsNumberLike(value))
    return ValueForm::kNumber;
  if (IsIdentifierLike(value))
    return ValueForm::kIdentifier;
  return ValueForm::kOther;
}

const char* TypeNameOf(ParamKind kind) {
  switch (kind) {
    case ParamKind::kText:
      return "文本";
    case ParamKind::kBool:
      return "真/假";
    case ParamKind::kTypeName:
      return "类型名称";
    case ParamKind::kMemberName:
      return "成员名称";
    default:
      return "";
  }
}

std::size_t TextLength(const std::string& text) {
  std::size_t count = 0;
  for (char c : text) {
    if ((static_cast<unsigned char>(c) & 0xC0) != 0x80)
      ++count;
  }
  return count;
}

SourcePosition ArgumentValuePosition(const AnnotationArgument& argument) {
  SourcePosition position = argument.position;
  if (!argument.name.empty())
    position.column += TextLength(argument.name) + TextLength(" = ");
  return position;
}

bool ValidateAnnotationArguments(const Annotation& annotation, const AnnotationRule& rule,
                                 const SourcePosition& target_position, const std::string& file,
                                 const AnnotationSymbolIndex& index, const std::string& owner_class,
                                 Diagnostics* diagnostics, bool* had_error) {
  const int error_count_before = diagnostics->error_count();
  bool type_error_hit = false;
  const auto report_type_error = [&](const SourcePosition& position, ParamKind kind) {
    diagnostics->Error(file, position,
                       std::string("注解参数类型错误，所需类型为: \"") + TypeNameOf(kind) + "\"",
                       "parse.annotation.argument-type");
    type_error_hit = true;
  };
  if (rule.positional.empty() && rule.named.empty() && !annotation.arguments.empty()) {
    diagnostics->Error(file, annotation.symbol_position,
                       "注解 \"" + annotation.name + "\" 不需要任何参数",
                       "parse.annotation.unexpected-argument");
    if (had_error != nullptr)
      *had_error = diagnostics->error_count() > error_count_before;
    return true;
  }
  struct EffectiveArgument {
    const AnnotationArgument* argument = nullptr;
    const ParamRule* param = nullptr;
  };
  std::vector<EffectiveArgument> effective;
  std::size_t positional_index = 0;
  for (const AnnotationArgument& argument : annotation.arguments) {
    const ParamRule* param = nullptr;
    if (argument.name.empty()) {
      if (positional_index < rule.positional.size())
        param = &rule.positional[positional_index];
      ++positional_index;
    } else {
      param = FindParam(rule.positional, argument.name);
      if (param == nullptr)
        param = FindParam(rule.named, argument.name);
    }
    if (param == nullptr || param->kind == ParamKind::kAny) {
      effective.push_back({&argument, param});
      continue;
    }
    if (param->kind == ParamKind::kCondition) {
      effective.push_back({&argument, param});
      continue;
    }
    const ValueForm form = ClassifyValue(argument);
    bool dropped = false;
    switch (param->kind) {
      case ParamKind::kText:
        if (form == ValueForm::kString || form == ValueForm::kOther) {
        } else if (form == ValueForm::kIdentifier) {
          const bool qualified = argument.value.find('.') != std::string::npos;
          if (index.ResolveTypeName(argument.value)) {
            report_type_error(annotation.position, param->kind);
          } else if (qualified && index.ResolveSymbol(argument.value)) {
          } else {
            diagnostics->Error(file, argument.position,
                               "无法将属性值解析为 常量值/类型名称/符号名称",
                               "parse.annotation.unresolved-value");
            dropped = true;
          }
        } else {
          report_type_error(annotation.position, param->kind);
        }
        break;
      case ParamKind::kTly:
        if (form != ValueForm::kOther) {
          report_type_error(annotation.position, param->kind);
        }
        break;
      case ParamKind::kBool:
        if (form != ValueForm::kBool && form != ValueForm::kOther) {
          report_type_error(annotation.position, param->kind);
        }
        break;
      case ParamKind::kTypeName:
        if (form == ValueForm::kIdentifier) {
          if (!argument.name.empty() || !index.ResolveTypeName(argument.value)) {
            diagnostics->Error(file, argument.position,
                               "无法将属性值解析为 常量值/类型名称/符号名称",
                               "parse.annotation.unresolved-value");
            dropped = true;
          }
        } else if (form != ValueForm::kOther) {
          report_type_error(annotation.position, param->kind);
        }
        break;
      case ParamKind::kMemberName:
        if (form == ValueForm::kIdentifier) {
          const bool is_member = index.ResolveMemberName(argument.value, owner_class);
          if (!is_member && index.ResolveTypeName(argument.value)) {
            report_type_error(annotation.position, param->kind);
          } else if (!is_member) {
            diagnostics->Error(file, ArgumentValuePosition(argument),
                               "未知符号: \"" + argument.value + "\"",
                               "parse.annotation.unknown-symbol");
            diagnostics->Error(file, argument.position,
                               "无法将属性值解析为 常量值/类型名称/符号名称",
                               "parse.annotation.unresolved-value");
            dropped = true;
          }
        } else if (form != ValueForm::kOther) {
          report_type_error(annotation.position, param->kind);
        }
        break;
      default:
        break;
    }
    if (!dropped)
      effective.push_back({&argument, param});
  }

  std::size_t positional_count = 0;
  std::set<std::string> provided_named;
  for (const EffectiveArgument& item : effective) {
    if (item.argument->name.empty()) {
      ++positional_count;
    } else {
      provided_named.insert(item.argument->name);
    }
  }
  for (std::size_t i = positional_count; i < rule.positional.size(); ++i) {
    if (rule.positional[i].optional)
      continue;
    if (provided_named.count(rule.positional[i].name) != 0)
      continue;
    if (rule.positional[i].kind == ParamKind::kCondition) {
      diagnostics->Error(file, annotation.symbol_position,
                         "缺少注解参数: \"" + rule.positional[i].name + "\"",
                         "parse.annotation.missing-argument");
      type_error_hit = true;
      continue;
    }
    diagnostics->Error(file, target_position, "缺少注解参数: \"" + rule.positional[i].name + "\"",
                       "parse.annotation.missing-argument");
    type_error_hit = true;
  }
  for (const ParamRule& param : rule.named) {
    if (!param.required || provided_named.count(param.name) != 0)
      continue;
    diagnostics->Error(file, target_position, "缺少注解参数: \"" + param.name + "\"",
                       "parse.annotation.missing-argument");
    type_error_hit = true;
  }
  for (const std::string& name : provided_named) {
    bool known =
        FindParam(rule.positional, name) != nullptr || FindParam(rule.named, name) != nullptr;
    if (known)
      continue;
    diagnostics->Error(file, annotation.position, "未知注解参数: \"" + name + "\"",
                       "parse.annotation.unknown-argument");
  }
  for (const EffectiveArgument& item : effective) {
    if (item.param == nullptr || item.param->kind != ParamKind::kCondition)
      continue;
    std::string expression;
    for (const EffectiveArgument& condition_arg : effective) {
      if (condition_arg.param != nullptr && condition_arg.param->kind == ParamKind::kCondition)
        expression += condition_arg.argument->value;
    }
    if (expression.empty())
      continue;
    std::map<std::string, bool> no_defines;
    if (!CompileConditionParses(expression, no_defines)) {
      diagnostics->Error(file, item.argument->position, "无法解析编译条件",
                         "parse.annotation.condition");
    }
    break;
  }
  if (had_error != nullptr) {
    *had_error = diagnostics->error_count() > error_count_before;
  }
  return type_error_hit;
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

std::string SimpleTypeName(const std::string& type_name) {
  const std::string base = StripGenerics(type_name);
  const std::size_t dot = base.rfind('.');
  return dot == std::string::npos ? base : base.substr(dot + 1);
}

void ValidateTlyVariableAnnotations(const Member& member, const std::string& file,
                                    const AnnotationSymbolIndex& index,
                                    const std::set<const Annotation*>& type_error_annotations,
                                    Diagnostics* diagnostics) {
  if (member.kind != MemberKind::kVariable || diagnostics == nullptr)
    return;
  const auto find_tly_annotation = [&member, &type_error_annotations](
                                       const std::string& name,
                                       bool allow_text) -> const Annotation* {
    for (const Annotation& annotation : member.annotations) {
      if (annotation.name != name)
        continue;
      if (type_error_annotations.count(&annotation) != 0)
        return nullptr;
      for (const AnnotationArgument& argument : annotation.arguments) {
        if (argument.value.find('{') != std::string::npos)
          return &annotation;
        if (allow_text && argument.is_string)
          return &annotation;
      }
      return nullptr;
    }
    return nullptr;
  };
  if (const Annotation* annotation = find_tly_annotation("组件配置", false)) {
    if (!index.ClassChainContains(member.type_name, kWindowComponentClass)) {
      diagnostics->Error(file, annotation->symbol_position,
                         "\"组件配置\" 只能应用在窗口组件类型的变量上",
                         "parse.annotation.component-variable");
    } else {
      for (const AnnotationArgument& argument : annotation->arguments) {
        if (argument.value.find('{') == std::string::npos)
          continue;
        for (const std::string& key : TlyAttributeKeys(argument.value)) {
          if (!key.empty() && key.front() == '$')
            continue;
          if (index.ResolveMemberName(key, StripGenerics(member.type_name)))
            continue;
          diagnostics->Error(
              file, annotation->symbol_position,
              "类\"" + SimpleTypeName(member.type_name) + "\"中找不到属性\"" + key + "\"",
              "parse.annotation.unknown-attribute");
          break;
        }
        break;
      }
    }
  }
  if (const Annotation* annotation = find_tly_annotation("主题配置", false)) {
    if (index.QualifiedNameOf(StripGenerics(member.type_name)) != "结绳.安卓.主题资源") {
      diagnostics->Error(file, annotation->symbol_position,
                         "\"主题配置\" 只能应用在 \"主题资源\" 类型的变量上",
                         "parse.annotation.theme-variable");
    }
  }
  if (const Annotation* annotation = find_tly_annotation("布局配置", true)) {
    if (!index.ClassChainContains(member.type_name, kLayoutComponentClass)) {
      diagnostics->Error(file, annotation->symbol_position,
                         "\"布局配置\" 中父布局必须为布局组件成员变量",
                         "parse.annotation.layout-variable");
    }
  }
}

void ValidateAnnotationList(const std::vector<Annotation>& annotations, int level,
                            const SourcePosition& target_position, const std::string& file,
                            const AnnotationSymbolIndex& index, const std::string& owner_class,
                            Diagnostics* diagnostics,
                            std::set<const Annotation*>* invalid_annotations,
                            std::set<const Annotation*>* type_error_annotations) {
  if (diagnostics == nullptr)
    return;
  for (const Annotation& annotation : annotations) {
    const auto found = AnnotationRules().find(annotation.name);
    if (found == AnnotationRules().end()) {
      diagnostics->Error(file, annotation.position, "未知注解名称 \"" + annotation.name + "\"",
                         "parse.annotation.unknown");
      if (invalid_annotations != nullptr)
        invalid_annotations->insert(&annotation);
      continue;
    }
    if ((found->second.levels & level) == 0) {
      diagnostics->Error(
          file, annotation.position,
          "注解\"" + annotation.name + "\"不能标注在\"" + LevelName(level) + "\"级别上",
          "parse.annotation.target");
      if (invalid_annotations != nullptr)
        invalid_annotations->insert(&annotation);
      continue;
    }
    bool had_error = false;
    const bool type_error =
        ValidateAnnotationArguments(annotation, found->second, target_position, file, index,
                                    owner_class, diagnostics, &had_error);
    if (type_error && type_error_annotations != nullptr)
      type_error_annotations->insert(&annotation);
    if (had_error && invalid_annotations != nullptr)
      invalid_annotations->insert(&annotation);
  }
}

void ValidateFileAnnotation(const Annotation& annotation, const std::string& file,
                            const std::vector<std::string>& prefixes, Diagnostics* diagnostics) {
  const AnnotationArgument* argument = nullptr;
  for (const AnnotationArgument& candidate : annotation.arguments) {
    if (candidate.name.empty() || candidate.name == kArgumentParameter) {
      argument = &candidate;
      break;
    }
  }
  if (argument == nullptr)
    return;
  if (!annotation.arguments.empty() && argument != &annotation.arguments.back())
    return;
  const std::string path = Unquote(argument->value);
  if (path.empty())
    return;
  if (file_search::ResolveWithPrefixes(path, file, prefixes, nullptr))
    return;
  diagnostics->Error(file, argument->position,
                     "未找到文件(夹)\"" + path + "\"，请检查相应文件是否存在或是否具有访问权限",
                     "parse.annotation.missing-file");
}

const std::vector<std::string>* FilePrefixesFor(const std::string& name,
                                                const std::vector<std::string>& source,
                                                const std::vector<std::string>& res,
                                                const std::vector<std::string>& lib,
                                                const std::vector<std::string>& assets) {
  (void)assets;
  if (name == annotation::kExternalJavaFile)
    return &source;
  if (name == annotation::kAndroidResource || name == "安卓资源.图片" || name == "安卓资源.动画" ||
      name == "安卓资源.布局" || name == "安卓资源.XML") {
    return &res;
  }
  if (name == "外部依赖库" || name == "外部动态库")
    return &lib;
  return nullptr;
}

}

void ValidateAnnotations(const Unit& unit, const AnnotationSymbolIndex& index,
                         Diagnostics* diagnostics,
                         std::set<const Annotation*>* invalid_annotations) {
  const SourcePosition unit_position{0, 0};
  std::set<const Annotation*> type_error_annotations;
  ValidateAnnotationList(unit.annotations, kLevelUnit, unit_position, unit.file, index,
                         std::string(), diagnostics, invalid_annotations, &type_error_annotations);
  for (const ClassDecl& class_decl : unit.classes) {
    ValidateAnnotationList(class_decl.annotations, kLevelClass, class_decl.keyword_position,
                           unit.file, index, class_decl.name, diagnostics, invalid_annotations,
                           &type_error_annotations);
    for (const Member& member : class_decl.members) {
      if (member.kind == MemberKind::kEventBlock || member.kind == MemberKind::kCodeBlock) {
        continue;
      }
      int level = kLevelVariable;
      if (member.kind == MemberKind::kMethod || member.kind == MemberKind::kPropertyRead ||
          member.kind == MemberKind::kPropertyWrite ||
          member.kind == MemberKind::kEventDefinition) {
        level = kLevelMethod;
      }
      ValidateAnnotationList(member.annotations, level, member.position, unit.file, index,
                             class_decl.name, diagnostics, invalid_annotations,
                             &type_error_annotations);
      ValidateTlyVariableAnnotations(member, unit.file, index, type_error_annotations, diagnostics);
    }
  }
}

void ValidateAnnotationFiles(const Unit& unit, const std::vector<std::string>& source_prefixes,
                             const std::vector<std::string>& res_prefixes,
                             const std::vector<std::string>& lib_prefixes,
                             const std::vector<std::string>& assets_prefixes,
                             Diagnostics* diagnostics,
                             const std::set<const Annotation*>* invalid_annotations) {
  if (diagnostics == nullptr)
    return;
  for (const ClassDecl& class_decl : unit.classes) {
    for (const Annotation& annotation : class_decl.annotations) {
      if (invalid_annotations != nullptr && invalid_annotations->count(&annotation) != 0) {
        continue;
      }
      const std::vector<std::string>* prefixes = FilePrefixesFor(
          annotation.name, source_prefixes, res_prefixes, lib_prefixes, assets_prefixes);
      if (prefixes == nullptr)
        continue;
      ValidateFileAnnotation(annotation, unit.file, *prefixes, diagnostics);
    }
  }
}

namespace {

bool IsForbiddenCreationType(const AnnotationSymbolIndex& index, const std::string& type_name) {
  if (type_name.find('[') != std::string::npos)
    return false;
  return index.forbidden_creation.count(SimpleTypeName(type_name)) != 0;
}

void CheckBlockForbiddenCreation(const std::vector<StmtPtr>& block, const std::string& file,
                                 const AnnotationSymbolIndex& index, Diagnostics* diagnostics) {
  for (const StmtPtr& statement : block) {
    if (statement == nullptr)
      continue;
    if (statement->kind == StmtKind::kVariable && statement->expr == nullptr &&
        IsForbiddenCreationType(index, statement->type_name)) {
      diagnostics->Error(file, statement->position,
                         "变量\"" + statement->name +
                             "\"类型标注了禁止创建对象，编译器无法为其自动创建对象，"
                             "请将其定义为参考变量",
                         "semantic.forbidden-creation");
    }
    CheckBlockForbiddenCreation(statement->body, file, index, diagnostics);
    CheckBlockForbiddenCreation(statement->else_body, file, index, diagnostics);
    for (const SwitchCase& item : statement->cases)
      CheckBlockForbiddenCreation(item.body, file, index, diagnostics);
  }
}

}

void ValidateForbiddenCreation(const Unit& unit, const AnnotationSymbolIndex& index,
                               Diagnostics* diagnostics) {
  if (diagnostics == nullptr)
    return;
  for (const ClassDecl& class_decl : unit.classes) {
    for (const Member& member : class_decl.members) {
      if (member.kind == MemberKind::kVariable && member.expression_body == nullptr &&
          IsForbiddenCreationType(index, member.type_name)) {
        diagnostics->Error(unit.file, member.position,
                           "变量\"" + member.name +
                               "\"类型标注了禁止创建对象，编译器无法为其自动创建对象，"
                               "请将其定义为参考变量",
                           "semantic.forbidden-creation");
      }
      CheckBlockForbiddenCreation(member.body, unit.file, index, diagnostics);
    }
  }
}

}
