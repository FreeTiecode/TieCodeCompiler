#include "annotation_validation.h"

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

const char* LevelName(AnnotationLevel level) {
  switch (level) {
    case AnnotationLevel::kUnit:
      return "包名";
    case AnnotationLevel::kClass:
      return "类";
    case AnnotationLevel::kVariable:
      return "变量";
    case AnnotationLevel::kMethod:
      return "方法";
  }
  return "";
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

bool ValidateAnnotationArguments(const Annotation& annotation, const AnnotationMetadata& rule,
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

std::string SimpleTypeName(const std::string& type_name) {
  const std::string base = StripGenerics(type_name);
  const std::size_t dot = base.rfind('.');
  return dot == std::string::npos ? base : base.substr(dot + 1);
}

void ValidateAnnotationList(const std::vector<Annotation>& annotations, const DeclRef& declaration,
                            const SourcePosition& target_position, const std::string& file,
                            const AnnotationSymbolIndex& index, const std::string& owner_class,
                            const AnnotationRegistry& registry, const TargetKey& target,
                            const ValidationEnvironment& environment, Diagnostics* diagnostics,
                            std::set<const Annotation*>* invalid_annotations,
                            std::set<const Annotation*>* type_error_annotations) {
  if (diagnostics == nullptr)
    return;
  for (const Annotation& annotation : annotations) {
    const AnnotationProcessor* processor = registry.Find(annotation.name, target);
    if (processor == nullptr) {
      if (registry.HasName(annotation.name)) {
        diagnostics->Error(file, annotation.position,
                           "注解\"" + annotation.name + "\"不适用于当前目标",
                           "parse.annotation.target-unsupported");
      } else {
        diagnostics->Error(file, annotation.position, "未知注解名称 \"" + annotation.name + "\"",
                           "parse.annotation.unknown");
      }
      if (invalid_annotations != nullptr)
        invalid_annotations->insert(&annotation);
      continue;
    }
    const AnnotationMetadata metadata = processor->Metadata();
    if ((metadata.levels & static_cast<unsigned>(declaration.level)) == 0) {
      diagnostics->Error(
          file, annotation.position,
          "注解\"" + annotation.name + "\"不能标注在\"" + LevelName(declaration.level) + "\"级别上",
          "parse.annotation.target");
      if (invalid_annotations != nullptr)
        invalid_annotations->insert(&annotation);
      continue;
    }
    const int error_count_before = diagnostics->error_count();
    bool had_error = false;
    const bool type_error = ValidateAnnotationArguments(
        annotation, metadata, target_position, file, index, owner_class, diagnostics, &had_error);
    ValidationContext context{target,
                              index,
                              environment,
                              *diagnostics,
                              *declaration.unit,
                              declaration.klass,
                              declaration.member,
                              owner_class};
    context.type_error = type_error;
    processor->Validate(annotation, declaration, context);
    if (diagnostics->error_count() > error_count_before)
      had_error = true;
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

const std::vector<std::string>* PrefixesFor(FileSearchKind kind,
                                            const ValidationEnvironment& environment) {
  switch (kind) {
    case FileSearchKind::kSource:
      return &environment.source_prefixes;
    case FileSearchKind::kResource:
      return &environment.res_prefixes;
    case FileSearchKind::kLibrary:
      return &environment.lib_prefixes;
    case FileSearchKind::kAsset:
      return &environment.assets_prefixes;
    case FileSearchKind::kNone:
      break;
  }
  return nullptr;
}

}

void ValidateAnnotations(const Unit& unit, const AnnotationSymbolIndex& index,
                         const AnnotationRegistry& registry, const TargetKey& target,
                         const ValidationEnvironment& environment, Diagnostics* diagnostics,
                         std::set<const Annotation*>* invalid_annotations) {
  const SourcePosition unit_position{0, 0};
  std::set<const Annotation*> type_error_annotations;
  const DeclRef unit_declaration{&unit, nullptr, nullptr, nullptr, AnnotationLevel::kUnit};
  ValidateAnnotationList(unit.annotations, unit_declaration, unit_position, unit.file, index,
                         std::string(), registry, target, environment, diagnostics,
                         invalid_annotations, &type_error_annotations);
  for (const ClassDecl& class_decl : unit.classes) {
    const DeclRef class_declaration{&unit, &class_decl, nullptr, nullptr, AnnotationLevel::kClass};
    ValidateAnnotationList(class_decl.annotations, class_declaration, class_decl.keyword_position,
                           unit.file, index, class_decl.name, registry, target, environment,
                           diagnostics, invalid_annotations, &type_error_annotations);
    for (const Member& member : class_decl.members) {
      if (member.kind == MemberKind::kEventBlock || member.kind == MemberKind::kCodeBlock) {
        continue;
      }
      AnnotationLevel level = AnnotationLevel::kVariable;
      if (member.kind == MemberKind::kMethod || member.kind == MemberKind::kPropertyRead ||
          member.kind == MemberKind::kPropertyWrite ||
          member.kind == MemberKind::kEventDefinition) {
        level = AnnotationLevel::kMethod;
      }
      const DeclRef member_declaration{&unit, &class_decl, &member, nullptr, level};
      ValidateAnnotationList(member.annotations, member_declaration, member.position, unit.file,
                             index, class_decl.name, registry, target, environment, diagnostics,
                             invalid_annotations, &type_error_annotations);
    }
  }
}

void ValidateAnnotationFiles(const Unit& unit, const AnnotationRegistry& registry,
                             const TargetKey& target, const ValidationEnvironment& environment,
                             Diagnostics* diagnostics,
                             const std::set<const Annotation*>* invalid_annotations) {
  if (diagnostics == nullptr)
    return;
  for (const ClassDecl& class_decl : unit.classes) {
    for (const Annotation& annotation : class_decl.annotations) {
      if (invalid_annotations != nullptr && invalid_annotations->count(&annotation) != 0)
        continue;
      const AnnotationProcessor* processor = registry.Find(annotation.name, target);
      if (processor == nullptr)
        continue;
      const std::vector<std::string>* prefixes =
          PrefixesFor(processor->Metadata().file_kind, environment);
      if (prefixes == nullptr)
        continue;
      ValidateFileAnnotation(annotation, unit.file, *prefixes, diagnostics);
    }
  }
}

namespace {

bool IsForbiddenCreationType(const std::set<std::string>& forbidden, const std::string& type_name) {
  if (type_name.find('[') != std::string::npos)
    return false;
  return forbidden.count(SimpleTypeName(type_name)) != 0;
}

void CheckBlockForbiddenCreation(const std::vector<StmtPtr>& block, const std::string& file,
                                 const std::set<std::string>& forbidden, Diagnostics* diagnostics) {
  for (const StmtPtr& statement : block) {
    if (statement == nullptr)
      continue;
    if (statement->kind == StmtKind::kVariable && statement->expr == nullptr &&
        IsForbiddenCreationType(forbidden, statement->type_name)) {
      diagnostics->Error(file, statement->position,
                         "变量\"" + statement->name +
                             "\"类型标注了禁止创建对象，编译器无法为其自动创建对象，"
                             "请将其定义为参考变量",
                         "semantic.forbidden-creation");
    }
    CheckBlockForbiddenCreation(statement->body, file, forbidden, diagnostics);
    CheckBlockForbiddenCreation(statement->else_body, file, forbidden, diagnostics);
    for (const SwitchCase& item : statement->cases)
      CheckBlockForbiddenCreation(item.body, file, forbidden, diagnostics);
  }
}

}

void ValidateForbiddenCreation(const std::vector<Unit>& units, const SemanticModel& semantics,
                               Diagnostics* diagnostics) {
  if (diagnostics == nullptr)
    return;
  std::set<std::string> forbidden;
  for (const Unit& unit : units) {
    for (const ClassDecl& class_decl : unit.classes) {
      const ClassFacts* facts = semantics.FindClass(&class_decl);
      if (facts != nullptr && facts->forbid_new)
        forbidden.insert(class_decl.name);
    }
  }
  for (const Unit& unit : units) {
    for (const ClassDecl& class_decl : unit.classes) {
      for (const Member& member : class_decl.members) {
        if (member.kind == MemberKind::kVariable && member.expression_body == nullptr &&
            IsForbiddenCreationType(forbidden, member.type_name)) {
          diagnostics->Error(unit.file, member.position,
                             "变量\"" + member.name +
                                 "\"类型标注了禁止创建对象，编译器无法为其自动创建对象，"
                                 "请将其定义为参考变量",
                             "semantic.forbidden-creation");
        }
        CheckBlockForbiddenCreation(member.body, unit.file, forbidden, diagnostics);
      }
    }
  }
}

}
