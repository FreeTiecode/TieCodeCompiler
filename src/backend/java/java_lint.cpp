#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "java_const.h"
#include "java_emitter.h"

#include <filesystem>
#include <regex>
#include "constants.h"
#include "file_search.h"
#include "java_names.h"
#include "symbol_resolution.h"
#include "text_util.h"
#include "unused_locals.h"

namespace tiecodec {
namespace {

bool HasAnnotationName(const std::vector<Annotation>& annotations, const std::string& name) {
  for (const Annotation& annotation : annotations) {
    if (annotation.name == name)
      return true;
  }
  return false;
}

bool ListAlwaysReturns(const std::vector<StmtPtr>& statements);

bool type_name_contains_array(const std::string& type_name) {
  return type_name.find('[') != std::string::npos;
}

std::string StripTypeDecorations(const std::string& type_name) {
  std::string result;
  int depth = 0;
  for (char c : type_name) {
    if (c == '<') {
      ++depth;
      continue;
    }
    if (c == '>') {
      --depth;
      continue;
    }
    if (depth > 0)
      continue;
    if (c == '[' || c == '?')
      break;
    result.push_back(c);
  }
  return result;
}

bool ContainsCodeStatement(const std::vector<StmtPtr>& statements) {
  for (const StmtPtr& statement : statements) {
    if (statement == nullptr)
      continue;
    if (statement->kind == StmtKind::kCode)
      return true;
    if (ContainsCodeStatement(statement->body))
      return true;
    if (ContainsCodeStatement(statement->else_body))
      return true;
    for (const SwitchCase& switch_case : statement->cases) {
      if (ContainsCodeStatement(switch_case.body))
        return true;
    }
  }
  return false;
}

bool StatementAlwaysReturns(const Stmt& statement) {
  switch (statement.kind) {
    case StmtKind::kReturn:
      return true;
    case StmtKind::kBlock:
      return ListAlwaysReturns(statement.body);
    case StmtKind::kIf:
      return !statement.else_body.empty() && ListAlwaysReturns(statement.body) &&
             ListAlwaysReturns(statement.else_body);
    case StmtKind::kSwitch: {
      bool has_default = false;
      for (const SwitchCase& switch_case : statement.cases) {
        if (switch_case.values.empty())
          has_default = true;
        if (!ListAlwaysReturns(switch_case.body))
          return false;
      }
      return has_default;
    }
    default:
      return false;
  }
}

bool ListAlwaysReturns(const std::vector<StmtPtr>& statements) {
  for (const StmtPtr& statement : statements) {
    if (statement != nullptr && StatementAlwaysReturns(*statement))
      return true;
  }
  return false;
}

std::string PrimitiveNameOf(const ClassInfo& info) {
  if (info.declaration == nullptr)
    return {};
  for (const Annotation& annotation : info.declaration->annotations) {
    if (annotation.name != "指代类")
      continue;
    const std::string value = annotation.TextArgument(kArgumentParameter);
    if (!value.empty())
      return value;
  }
  return {};
}

std::string BaseTypeName(const std::string& type_name) {
  std::string base = type_name;
  if (!base.empty() && base.back() == '?')
    base.pop_back();
  const std::size_t bracket = base.find('[');
  if (bracket != std::string::npos)
    base = base.substr(0, bracket);
  const std::size_t angle = base.find('<');
  if (angle != std::string::npos)
    base = base.substr(0, angle);
  return base;
}

int PrimitiveRank(const std::string& name) {
  if (name == "byte" || name == "short" || name == "int" || name == "char")
    return 0;
  if (name == "long")
    return 1;
  if (name == "float")
    return 2;
  if (name == "double")
    return 3;
  return -1;
}

int PrimitiveCategory(const std::string& name) {
  const int rank = PrimitiveRank(name);
  if (rank >= 0)
    return 0;
  if (name == "boolean")
    return 1;
  if (name == "java.lang.String" || name == "String" || name == "char" ||
      name == "java.lang.Character") {
    return 2;
  }
  return -1;
}

}

bool JavaEmitter::LintDisabled(const std::string& key) const {
  const std::string list = config_.Option(option::kDisableLint);
  if (list.empty())
    return false;
  if (list == "all")
    return true;
  for (const std::string& item : SplitDelimited(list, ',')) {
    if (item == key)
      return true;
  }
  return false;
}

bool JavaEmitter::HasStaticAnnotation(const Member& member) const {
  return HasAnnotationName(member.annotations, annotation::kStatic);
}

bool JavaEmitter::HasAnnotationMember(const Member& member, const std::string& name) const {
  return HasAnnotationName(member.annotations, name);
}

void JavaEmitter::LintStaticAccess(const ClassInfo& klass, const SourcePosition& position,
                                   const std::string& name) {
  if (LintDisabled("object.access-static") || klass.declaration == nullptr)
    return;
  diagnostics_.Warning(klass.declaration->file, position,
                       "成员\"" + name + "\"是静态的，建议直接使用类名进行访问",
                       "object.access-static");
}

bool JavaEmitter::IsResourceType(const ClassInfo& info) const {
  if (info.declaration == nullptr)
    return false;
  for (const Annotation& annotation : info.declaration->annotations) {
    if (annotation.name == annotation::kValueOutputRule ||
        annotation.name == annotation::kRequiredValueType ||
        annotation.name == annotation::kConstantType ||
        annotation.name == annotation::kConstantValue) {
      return true;
    }
  }
  return false;
}

std::string JavaEmitter::BaseResolvableName(const std::string& type_name) const {
  return StripTypeDecorations(type_name);
}

bool JavaEmitter::TypesCompatible(const ClassInfo& klass, const std::string& target_type,
                                  const std::string& source_type) const {
  const std::string target = StripTypeDecorations(target_type);
  const std::string source = StripTypeDecorations(source_type);
  if (target.empty() || source.empty())
    return true;
  if (target == source)
    return true;
  if (type_name_contains_array(target_type) || type_name_contains_array(source_type))
    return true;
  const ClassInfo* target_class = symbols_.Resolve(target, &klass);
  const ClassInfo* source_class = symbols_.Resolve(source, &klass);
  if (target_class == nullptr || source_class == nullptr)
    return true;
  if (target_class == source_class)
    return true;
  if (IsResourceType(*target_class))
    return true;
  if (target_class->has_java_alias && target_class->java_alias == "java.lang.Object")
    return true;
  if (target_class->has_java_alias && target_class->java_alias == "Object")
    return true;
  const std::string target_primitive = PrimitiveNameOf(*target_class);
  const std::string source_primitive = PrimitiveNameOf(*source_class);
  if (!target_primitive.empty() && !source_primitive.empty()) {
    return PrimitiveRank(target_primitive) >= 0 && PrimitiveRank(source_primitive) >= 0;
  }
  if (DerivesFrom(symbols_, target_class, source_class) ||
      DerivesFrom(symbols_, source_class, target_class)) {
    return true;
  }
  return false;
}

bool JavaEmitter::IsUnsafeCast(const ClassInfo& klass, const std::string& source_type,
                               const std::string& target_type) const {
  if (source_type.empty() || target_type.empty())
    return false;
  if (source_type == target_type)
    return false;
  const std::string source_base = BaseTypeName(source_type);
  const std::string target_base = BaseTypeName(target_type);
  const ClassInfo* source = symbols_.Resolve(source_base, &klass);
  const ClassInfo* target = symbols_.Resolve(target_base, &klass);
  if (source == nullptr || target == nullptr)
    return false;
  if (source == target)
    return false;
  const int source_rank = PrimitiveRank(PrimitiveNameOf(*source));
  const int target_rank = PrimitiveRank(PrimitiveNameOf(*target));
  if (source_rank >= 0 && target_rank >= 0)
    return source_rank > target_rank;
  if (source_rank >= 0 || target_rank >= 0)
    return false;
  return DerivesFrom(symbols_, target, source);
}

const ClassInfo* JavaEmitter::LintReceiverClass(const ClassInfo& klass,
                                                const Expr& receiver) const {
  switch (receiver.kind) {
    case ExprKind::kIdentifier: {
      const auto local = lint_types_.find(receiver.text);
      if (local != lint_types_.end())
        return symbols_.Resolve(local->second, &klass);
      for (const Parameter& parameter : lint_parameters_) {
        if (parameter.name == receiver.text) {
          return symbols_.Resolve(parameter.type_name, &klass);
        }
      }
      const ClassInfo* owner = nullptr;
      if (const Member* member = FindMemberInChain(symbols_, klass, receiver.text, &owner)) {
        if (!member->type_name.empty()) {
          return symbols_.Resolve(member->type_name, &klass);
        }
      }
      if (const Member* global = FindGlobalClassMember(symbols_, klass, receiver.text, &owner)) {
        if (!global->type_name.empty()) {
          return symbols_.Resolve(global->type_name, &klass);
        }
      }
      return symbols_.Resolve(receiver.text, &klass);
    }
    case ExprKind::kThis:
      return &klass;
    case ExprKind::kSuper: {
      if (klass.declaration == nullptr || klass.declaration->base_name.empty())
        return nullptr;
      return symbols_.Resolve(klass.declaration->base_name, &klass);
    }
    case ExprKind::kNew:
    case ExprKind::kCast:
      return symbols_.Resolve(receiver.text, &klass);
    case ExprKind::kCall:
    case ExprKind::kFieldAccess:
    case ExprKind::kArrayAccess:
    case ExprKind::kParen: {
      const std::string type = InferTypeName(klass, receiver);
      if (type.empty())
        return nullptr;
      return symbols_.Resolve(type, &klass);
    }
    default:
      return nullptr;
  }
}

std::string JavaEmitter::InferTypeName(const ClassInfo& klass, const Expr& expression) const {
  switch (expression.kind) {
    case ExprKind::kLiteral: {
      if (expression.text == "真" || expression.text == "假")
        return builtin_type::kBoolean;
      if (!expression.raw.empty() && (expression.raw[0] == '"' || expression.raw[0] == '\'')) {
        return builtin_type::kText;
      }
      if (expression.text.size() > 1 && expression.text[0] == '0' &&
          (expression.text[1] == 'x' || expression.text[1] == 'X')) {
        return builtin_type::kInteger;
      }
      if (expression.text.find('.') != std::string::npos ||
          expression.text.find('e') != std::string::npos ||
          expression.text.find('E') != std::string::npos) {
        return builtin_type::kDecimal;
      }
      return builtin_type::kInteger;
    }
    case ExprKind::kIdentifier: {
      const auto local = lint_types_.find(expression.text);
      if (local != lint_types_.end())
        return local->second;
      if (!expression.text.empty()) {
        for (const Parameter& parameter : lint_parameters_) {
          if (parameter.name == expression.text)
            return parameter.type_name;
        }
      }
      const ClassInfo* owner = nullptr;
      if (const Member* member = FindMemberInChain(symbols_, klass, expression.text, &owner)) {
        return member->type_name;
      }
      if (const Member* global = FindGlobalClassMember(symbols_, klass, expression.text, &owner)) {
        return global->type_name;
      }
      return {};
    }
    case ExprKind::kThis:
      return klass.declaration != nullptr ? klass.declaration->name : std::string();
    case ExprKind::kFieldAccess: {
      if (expression.children.empty() || expression.children[0] == nullptr)
        return {};
      const std::string receiver_type = InferTypeName(klass, *expression.children[0]);
      if (!receiver_type.empty() && receiver_type.back() == ']') {
        return expression.text == "长度" ? std::string(builtin_type::kInteger) : std::string();
      }
      const ClassInfo* receiver = LintReceiverClass(klass, *expression.children[0]);
      if (receiver == nullptr)
        return {};
      const ClassInfo* owner = nullptr;
      if (const Member* member = FindMemberInChain(symbols_, *receiver, expression.text, &owner)) {
        return member->type_name;
      }
      return {};
    }
    case ExprKind::kCall: {
      if (expression.children.empty() || expression.children[0] == nullptr)
        return {};
      const ClassInfo* owner = nullptr;
      if (const Member* member = ResolveCalledMember(klass, *expression.children[0], &owner)) {
        return member->type_name;
      }
      return {};
    }
    case ExprKind::kNew:
      return expression.text;
    case ExprKind::kCast:
      return expression.text;
    case ExprKind::kArrayLiteral: {
      if (expression.children.empty())
        return {};
      std::string widest;
      for (const ExprPtr& child : expression.children) {
        if (child == nullptr)
          continue;
        const std::string type = InferTypeName(klass, *child);
        if (type.empty())
          continue;
        if (widest.empty()) {
          widest = type;
          continue;
        }
        if (widest == type)
          continue;
        const ClassInfo* widest_class = symbols_.Resolve(BaseTypeName(widest), &klass);
        const ClassInfo* type_class = symbols_.Resolve(BaseTypeName(type), &klass);
        const int widest_rank =
            widest_class != nullptr ? PrimitiveRank(PrimitiveNameOf(*widest_class)) : -1;
        const int type_rank =
            type_class != nullptr ? PrimitiveRank(PrimitiveNameOf(*type_class)) : -1;
        if (type_rank > widest_rank)
          widest = type;
      }
      if (widest.empty())
        return {};
      return widest + "[]";
    }
    case ExprKind::kArrayAccess: {
      if (expression.children.empty() || expression.children[0] == nullptr)
        return {};
      const std::string array_type = InferTypeName(klass, *expression.children[0]);
      if (array_type.empty())
        return {};
      const std::size_t bracket = array_type.find('[');
      if (bracket == std::string::npos) {
        const ClassInfo* receiver = symbols_.Resolve(BaseResolvableName(array_type), &klass);
        if (receiver == nullptr)
          return {};
        const ClassInfo* owner = nullptr;
        if (const Member* indexer =
                FindMemberInChain(symbols_, *receiver, kOperatorIndex, &owner)) {
          return indexer->type_name;
        }
        return {};
      }
      const std::size_t close = array_type.find(']', bracket);
      const std::string rest =
          close == std::string::npos ? std::string() : array_type.substr(close + 1);
      return BaseTypeName(array_type.substr(0, bracket)) + rest;
    }
    case ExprKind::kParen:
      if (expression.children.size() == 1 && expression.children[0] != nullptr) {
        return InferTypeName(klass, *expression.children[0]);
      }
      return {};
    case ExprKind::kUnary: {
      if (expression.text == "非")
        return builtin_type::kBoolean;
      if (!expression.children.empty() && expression.children[0] != nullptr) {
        return InferTypeName(klass, *expression.children[0]);
      }
      return {};
    }
    case ExprKind::kBinary: {
      const std::string& op = expression.text;
      if (op == "==" || op == "!=" || op == "<" || op == "<=" || op == ">" || op == ">=" ||
          op == "且" || op == "&&" || op == "或" || op == "||") {
        return builtin_type::kBoolean;
      }
      if (op == "+" || op == "&") {
        bool any_text = false;
        for (const ExprPtr& child : expression.children) {
          if (child == nullptr)
            continue;
          if (InferTypeName(klass, *child) == builtin_type::kText)
            any_text = true;
        }
        if (any_text)
          return builtin_type::kText;
      }
      bool any_double = false;
      bool any_long = false;
      for (const ExprPtr& child : expression.children) {
        if (child == nullptr)
          continue;
        const std::string type = InferTypeName(klass, *child);
        if (type == builtin_type::kDecimal)
          any_double = true;
        if (type == builtin_type::kLongInteger)
          any_long = true;
      }
      if (any_double)
        return builtin_type::kDecimal;
      if (any_long)
        return builtin_type::kLongInteger;
      return builtin_type::kInteger;
    }
    default:
      return {};
  }
}

void JavaEmitter::LintAutoCast(const ClassInfo& klass, const SourcePosition& position,
                               const std::string& target_type, const Expr* expression) {
  if (expression == nullptr || target_type.empty())
    return;
  if (LintDisabled("auto.type-cast"))
    return;
  if (klass.declaration == nullptr)
    return;
  const std::string source_type = InferTypeName(klass, *expression);
  if (!IsUnsafeCast(klass, source_type, target_type))
    return;
  (void)position;
  diagnostics_.Warning(klass.declaration->file, ExpressionStart(*expression),
                       "类型自动强制转换为\"" + target_type + "\"，运行时不一定安全",
                       "auto.type-cast");
}

void JavaEmitter::RunAnalysisLints() {
  if (diagnostics_.has_errors())
    return;
  bool orphan_found = false;
  {
    std::function<void(const std::vector<StmtPtr>&, const ClassInfo&)> walk =
        [&](const std::vector<StmtPtr>& statements, const ClassInfo& klass) {
          for (const StmtPtr& statement : statements) {
            if (statement == nullptr)
              continue;
            if (statement->kind == StmtKind::kExpression && statement->expr != nullptr) {
              switch (statement->expr->kind) {
                case ExprKind::kLiteral:
                case ExprKind::kIdentifier:
                case ExprKind::kBinary:
                case ExprKind::kUnary:
                case ExprKind::kParen:
                case ExprKind::kMulti:
                case ExprKind::kIsType:
                case ExprKind::kArrayAccess:
                case ExprKind::kFieldAccess:
                  if (klass.declaration != nullptr) {
                    diagnostics_.Error(klass.declaration->file, ExpressionStart(*statement->expr),
                                       "该表达式为孤立表达式，不是完整的表达式",
                                       "semantic.expr.orphan");
                  }
                  orphan_found = true;
                  break;
                default:
                  break;
              }
            }
            for (const SwitchCase& switch_case : statement->cases) {
              walk(switch_case.body, klass);
            }
            walk(statement->body, klass);
            walk(statement->else_body, klass);
          }
        };
    for (const std::string& class_name : symbols_.class_names()) {
      const auto found = symbols_.classes().find(class_name);
      if (found == symbols_.classes().end())
        continue;
      const ClassInfo& klass = found->second;
      if (klass.declaration == nullptr)
        continue;
      for (const Member& member : klass.declaration->members) {
        if (IsAsyncMember(member))
          continue;
        walk(member.body, klass);
      }
    }
  }
  if (orphan_found)
    return;
  {
    std::set<const ClassInfo*> reported_cycles;
    for (const std::string& class_name : symbols_.class_names()) {
      const auto found = symbols_.classes().find(class_name);
      if (found == symbols_.classes().end())
        continue;
      const ClassInfo& klass = found->second;
      if (klass.declaration == nullptr)
        continue;
      std::vector<const ClassInfo*> chain;
      std::set<const ClassInfo*> seen;
      const ClassInfo* cursor = &klass;
      while (cursor != nullptr && cursor->declaration != nullptr) {
        if (!seen.insert(cursor).second)
          break;
        chain.push_back(cursor);
        if (cursor->declaration->base_name.empty()) {
          cursor = nullptr;
          break;
        }
        cursor = symbols_.Resolve(BaseResolvableName(cursor->declaration->base_name), cursor);
      }
      if (cursor == nullptr || cursor->declaration == nullptr || chain.empty())
        continue;
      const ClassInfo* reporter = nullptr;
      for (const ClassInfo* node : chain) {
        if (node->declaration != nullptr &&
            (reporter == nullptr ||
             node->declaration->declaration_line > reporter->declaration->declaration_line)) {
          reporter = node;
        }
      }
      if (reporter == nullptr || reported_cycles.count(reporter) != 0)
        continue;
      const ClassInfo* base = nullptr;
      for (std::size_t i = 0; i < chain.size(); ++i) {
        if (chain[i] != reporter)
          continue;
        base = chain[(i + 1) % chain.size()];
      }
      if (base == nullptr)
        continue;
      reported_cycles.insert(reporter);
      diagnostics_.Error(reporter->declaration->file, reporter->declaration->keyword_position,
                         "类\"" + reporter->declaration->name + "\"和类\"" +
                             base->declaration->name + "\"循环继承",
                         "semantic.class.inherit-cycle");
      return;
    }
  }
  const auto check_type = [this](const ClassInfo& klass, const std::string& type_name,
                                 const SourcePosition& position) {
    if (klass.declaration == nullptr)
      return;
    const std::string base = BaseResolvableName(type_name);
    if (base.empty() || base == "void" || base == builtin_type::kAnyType ||
        base == builtin_type::kAnyTypeAlias || base == "类型" || base == builtin_type::kVariant) {
      return;
    }
    if (base.rfind("模板类型", 0) == 0)
      return;
    for (const std::string& generic : klass.declaration->generic_parameters) {
      if (generic == base)
        return;
    }
    const ClassInfo* resolved = symbols_.Resolve(base, &klass);
    if (resolved == nullptr) {
      diagnostics_.Error(klass.declaration->file, position, "找不到类\"" + base + "\"",
                         "semantic.type.unresolved");
      return;
    }
    if (resolved->declaration == nullptr)
      return;
    const std::vector<std::string>& parameters = resolved->declaration->generic_parameters;
    const std::vector<bool>& defaults = resolved->declaration->generic_parameter_defaults;
    const std::size_t open = type_name.find('<');
    if (open == std::string::npos) {
      bool all_default = true;
      for (std::size_t i = 0; i < parameters.size(); ++i) {
        const bool has_default = i < defaults.size() && defaults[i];
        if (!has_default)
          all_default = false;
      }
      if (!parameters.empty() && !all_default) {
        diagnostics_.Error(
            klass.declaration->file, position,
            "模板类型\"" + base + "\"并非所有模板参数都有默认类型，必须显式指定模板参数",
            "semantic.generic.arity");
      }
      return;
    }
    int depth = 1;
    std::size_t argument_count = 0;
    bool has_argument = false;
    for (std::size_t i = open + 1; i < type_name.size(); ++i) {
      const char c = type_name[i];
      if (c == '<') {
        ++depth;
        has_argument = true;
      } else if (c == '>') {
        --depth;
        if (depth == 0) {
          if (has_argument)
            ++argument_count;
          break;
        }
      } else if (c == ',' && depth == 1) {
        ++argument_count;
        has_argument = false;
      } else if (depth >= 1 && c != ' ') {
        has_argument = true;
      }
    }
    if (argument_count > parameters.size()) {
      diagnostics_.Error(klass.declaration->file, position, "模板类型\"" + base + "\"实现参数过多",
                         "semantic.generic.arity");
    } else {
      for (std::size_t i = argument_count; i < parameters.size(); ++i) {
        const bool has_default = i < defaults.size() && defaults[i];
        if (has_default)
          continue;
        diagnostics_.Error(klass.declaration->file, position,
                           "模板类型\"" + base + "\"第" + std::to_string(i + 1) + "个参数未实现",
                           "semantic.generic.arity");
        break;
      }
    }
  };
  for (const std::string& class_name : symbols_.class_names()) {
    const auto found = symbols_.classes().find(class_name);
    if (found == symbols_.classes().end())
      continue;
    const ClassInfo& klass = found->second;
    if (klass.declaration == nullptr)
      continue;
    if (!klass.declaration->base_name.empty() && klass.external_base.empty()) {
      const ClassInfo* base_class = symbols_.Resolve(klass.declaration->base_name, &klass);
      if (base_class != nullptr && base_class->declaration != nullptr &&
          base_class->declaration->HasAnnotation("禁止继承")) {
        diagnostics_.Error(klass.declaration->file, klass.declaration->base_position,
                           "类\"" + klass.declaration->base_name + "\"禁止被继承",
                           "semantic.class.forbid-inherit");
      }
      check_type(klass, klass.declaration->base_name, klass.declaration->base_position);
    }
    for (const Member& member : klass.declaration->members) {
      check_type(klass, member.type_name, member.type_position);
      for (const Parameter& parameter : member.parameters) {
        check_type(klass, parameter.type_name, parameter.type_position);
      }
      if (IsAsyncMember(member)) {
        if (!member.type_name.empty()) {
          diagnostics_.Error(klass.declaration->file, member.name_position,
                             "异步方法不能有返回值类型", "semantic.async.return-type");
        }
        continue;
      }
      AnalyzeMember(klass, member);
    }
  }

  RemoveUnusedLocalsAfterLint();
}

void JavaEmitter::AnalyzeMember(const ClassInfo& klass, const Member& member) {
  if (klass.declaration == nullptr)
    return;
  const std::string file = klass.declaration->file;
  const auto warn = [this, &file](const SourcePosition& position, const std::string& message,
                                  const std::string& key) {
    if (!key.empty() && LintDisabled(key))
      return;
    diagnostics_.Warning(file, position, message, key);
  };
  const auto error = [this, &file](const SourcePosition& position, const std::string& message) {
    diagnostics_.Error(file, position, message, "semantic.type");
  };
  const auto primitive_mismatch = [this, &klass](const std::string& target_type,
                                                 const std::string& source_type) {
    const std::string target = StripTypeDecorations(target_type);
    const std::string source = StripTypeDecorations(source_type);
    if (target.empty() || source.empty())
      return false;
    if (type_name_contains_array(target_type) || type_name_contains_array(source_type)) {
      return false;
    }
    const ClassInfo* target_class = symbols_.Resolve(target, &klass);
    const ClassInfo* source_class = symbols_.Resolve(source, &klass);
    if (target_class == nullptr || source_class == nullptr)
      return false;
    const std::string target_primitive = PrimitiveNameOf(*target_class);
    const std::string source_primitive = PrimitiveNameOf(*source_class);
    if (target_primitive.empty() || source_primitive.empty())
      return false;
    const int target_category = PrimitiveCategory(target_primitive);
    const int source_category = PrimitiveCategory(source_primitive);
    if (target_category < 0 || source_category < 0)
      return false;
    return target_category != source_category;
  };
  const auto check_type = [this, &klass, &error](const std::string& type_name,
                                                 const SourcePosition& position) {
    if (klass.declaration == nullptr)
      return;
    const std::string base = BaseResolvableName(type_name);
    if (base.empty() || base == "void" || base == builtin_type::kAnyType ||
        base == builtin_type::kAnyTypeAlias || base == "类型" || base == builtin_type::kVariant) {
      return;
    }
    if (base.rfind("模板类型", 0) == 0)
      return;
    for (const std::string& generic : klass.declaration->generic_parameters) {
      if (generic == base)
        return;
    }
    if (symbols_.Resolve(base, &klass) != nullptr)
      return;
    error(position, "找不到类\"" + base + "\"");
  };

  const auto member_has_annotation = [&](const std::string& name) {
    for (const Annotation& annotation : member.annotations) {
      if (annotation.name == name)
        return true;
    }
    return false;
  };
  const auto class_annotation_text = [this](const ClassInfo& type, const std::string& name) {
    if (type.declaration == nullptr)
      return std::string();
    for (const Annotation& annotation : type.declaration->annotations) {
      if (annotation.name != name)
        continue;
      return annotation.TextArgument(kArgumentParameter);
    }
    return std::string();
  };
  const auto search_prefixes = [this]() { return config_.ListOption(option::kSearchResPrefix); };
  if ((member.kind == MemberKind::kVariable || member.kind == MemberKind::kConstant) &&
      member.expression_body != nullptr && member.expression_body->kind == ExprKind::kLiteral) {
    const ClassInfo* declared = symbols_.Resolve(BaseResolvableName(member.type_name), &klass);
    if (declared != nullptr && declared->declaration != nullptr) {
      const std::string rule = class_annotation_text(*declared, annotation::kValueOutputRule);
      if (rule == annotation::kRuleDrawable || rule == annotation::kRuleDrawableXxh ||
          rule == annotation::kRuleAnim || rule == annotation::kRuleXml) {
        const std::string value = Unquote(member.expression_body->text);
        if (!value.empty() && value.front() != '@') {
          const std::string source_file =
              klass.declaration != nullptr ? klass.declaration->file : std::string();
          const bool found_file =
              file_search::ResolveWithPrefixes(value, source_file, search_prefixes(), nullptr);
          const SourcePosition position = ExpressionStart(*member.expression_body);
          if (!found_file) {
            error(position,
                  "未找到文件(夹)\"" + value + "\"，请检查相应文件是否存在或是否具有访问权限");
          } else {
            const std::string pattern = class_annotation_text(*declared, annotation::kMatchFile);
            if (!pattern.empty()) {
              try {
                if (!std::regex_match(value, std::regex(pattern))) {
                  error(position, "文件格式不匹配\"" + pattern + "\"");
                }
              } catch (const std::regex_error&) {
              }
            }
          }
        }
      }
    }
  }

  const bool property_read =
      member.kind == MemberKind::kPropertyRead || member_has_annotation(annotation::kPropertyRead);
  const bool property_write = member.kind == MemberKind::kPropertyWrite ||
                              member_has_annotation(annotation::kPropertyWrite);
  if (property_read) {
    if (!member.parameters.empty()) {
      error(member.position,
            "属性读方法 \"" + member.name + "\" 不能具有参数，请删除该属性读方法的所有参数");
    }
    if (member.type_name.empty()) {
      error(member.position,
            "属性读方法 \"" + member.name + "\" 必须具有返回类型，请为该属性读方法增加返回类型");
    }
  }
  if (property_write) {
    if (member.parameters.size() != 1) {
      error(member.position, "属性写方法 \"" + member.name +
                                 "\" 必须具有且只能具有一个参数，请删除多余的参数或补充参数");
    }
    if (!member.type_name.empty()) {
      error(member.position,
            "属性写方法 \"" + member.name + "\" 不能具有返回类型，请删除该属性写方法的返回类型");
    }
  }
  if (member.name == kOperatorIndex && member.parameters.size() != 1) {
    error(member.position, "运算符重载方法必须具有且只能具有一个参数");
  }
  if (member.name == kOperatorIndexAssign && member.parameters.size() != 2) {
    error(member.position, "当重载运算符 \"[]=\" 时，该方法必须具有且只能具有两个参数");
  }
  if (member.name == "=" && klass.declaration != nullptr) {
    if (!HasStaticAnnotation(member) ||
        BaseResolvableName(member.type_name) != klass.declaration->name) {
      error(member.type_position, "重载\"=\"赋值符号方法返回类型必须为当前类\"" +
                                      klass.declaration->name + "\"，且必须为静态方法");
    }
  }
  for (const Annotation& annotation : member.annotations) {
    if (annotation.name != "隐式操作")
      continue;
    if (annotation.arguments.empty())
      continue;
    const std::string value = Unquote(annotation.arguments.front().value);
    if (value != "cast") {
      error(annotation.arguments.front().position, "隐式操作的类型目前只能为\"cast\"");
    }
  }
  lint_parameters_ = member.parameters;
  lint_types_.clear();
  lint_return_type_ = member.type_name;
  lint_returns_value_ =
      member.kind == MemberKind::kMethod || member.kind == MemberKind::kPropertyRead;

  std::set<std::string> reported_static;
  std::set<std::string> reported_hidden;
  const auto class_derives_from = [this](const ClassInfo& klass, const ClassInfo* base) {
    return DerivesFrom(symbols_, &klass, base);
  };
  const auto check_hidden = [&](const Member& member, const ClassInfo* owner,
                                const SourcePosition& position, bool inherited_access_allowed) {
    if (!HasAnnotationName(member.annotations, annotation::kHidden))
      return;
    if (owner == nullptr || owner == &klass)
      return;
    if (inherited_access_allowed && class_derives_from(klass, owner))
      return;
    const std::string dedupe =
        std::to_string(position.line) + ":" + std::to_string(position.column) + ":" + member.name;
    if (!reported_hidden.insert(dedupe).second)
      return;
    error(position, "成员\"" + member.name + "\"是隐藏的，只能在成员所处类中进行访问");
  };
  const auto check_call_arguments = [&](const ClassInfo& declaration_site, const Expr& call,
                                        const Expr& callee) {
    const std::size_t provided = call.children.size() > 0 ? call.children.size() - 1 : 0;
    std::string name;
    const ClassInfo* lookup_class = nullptr;
    if (callee.kind == ExprKind::kIdentifier) {
      name = callee.text;
      lookup_class = &klass;
    } else if (callee.kind == ExprKind::kFieldAccess && !callee.children.empty() &&
               callee.children[0] != nullptr) {
      name = callee.text;
      lookup_class = ReceiverClass(klass, *callee.children[0]);
    }
    if (lookup_class == nullptr || name.empty() || declaration_site.declaration == nullptr) {
      return;
    }
    std::vector<const Member*> candidates;
    const ClassInfo* cursor = lookup_class;
    std::set<const ClassInfo*> visited;
    while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
      for (const Member& candidate : cursor->declaration->members) {
        if (candidate.name == name &&
            (candidate.kind == MemberKind::kMethod || candidate.kind == MemberKind::kPropertyRead ||
             candidate.kind == MemberKind::kPropertyWrite)) {
          candidates.push_back(&candidate);
        }
      }
      if (cursor->declaration->base_name.empty())
        break;
      cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
    }
    if (candidates.empty())
      return;
    const auto acceptable = [&](const Member& candidate) {
      if (provided > candidate.parameters.size())
        return true;
      for (std::size_t i = provided; i < candidate.parameters.size(); ++i) {
        if (candidate.parameters[i].default_value == nullptr)
          return false;
      }
      return true;
    };
    for (const Member* candidate : candidates) {
      if (acceptable(*candidate))
        return;
    }
    const Member* reported = candidates.front();
    for (const Member* candidate : candidates) {
      if (candidate->parameters.size() > reported->parameters.size())
        reported = candidate;
    }
    const SourcePosition position = ExpressionStart(call);
    error(position, "方法\"" + reported->name + "\"需要" +
                        std::to_string(reported->parameters.size()) + "个参数，实际只传入了" +
                        std::to_string(provided) + "个参数");
  };

  std::function<void(const Expr*)> check_static = [&](const Expr* expression) {
    if (expression == nullptr)
      return;
    if (expression->kind == ExprKind::kCall && !expression->children.empty() &&
        expression->children[0] != nullptr &&
        expression->children[0]->kind == ExprKind::kIdentifier) {
      const ClassInfo* owner = nullptr;
      if (const Member* member_ptr = ResolveCalledMember(klass, *expression->children[0], &owner)) {
        check_call_arguments(klass, *expression, *expression->children[0]);
        check_hidden(*member_ptr, owner, ExpressionStart(*expression->children[0]), true);
      }
    }
    if (expression->kind == ExprKind::kCall && !expression->children.empty() &&
        expression->children[0] != nullptr &&
        expression->children[0]->kind == ExprKind::kFieldAccess) {
      const Expr& callee = *expression->children[0];
      if (!callee.children.empty() && callee.children[0] != nullptr) {
        const Expr& receiver = *callee.children[0];
        bool static_receiver = false;
        if (receiver.kind == ExprKind::kIdentifier) {
          const ClassInfo* resolved = symbols_.Resolve(receiver.text, &klass);
          const bool shadowed = lint_types_.count(receiver.text) != 0;
          static_receiver = resolved != nullptr && !shadowed;
        }
        if (!static_receiver) {
          const ClassInfo* owner = nullptr;
          if (const Member* member_ptr = ResolveCalledMember(klass, callee, &owner)) {
            check_call_arguments(klass, *expression, callee);
            const bool method_like = member_ptr->kind == MemberKind::kMethod ||
                                     member_ptr->kind == MemberKind::kPropertyRead ||
                                     member_ptr->kind == MemberKind::kPropertyWrite;
            check_hidden(*member_ptr, owner, ExpressionStart(receiver), method_like);
            if (HasStaticAnnotation(*member_ptr)) {
              const std::string dedupe = std::to_string(callee.member_position.line) + ":" +
                                         std::to_string(callee.member_position.column) + ":" +
                                         member_ptr->name;
              if (reported_static.insert(dedupe).second) {
                LintStaticAccess(klass, callee.member_position, member_ptr->name);
              }
            }
          }
        }
      }
    }
    if (expression->kind == ExprKind::kFieldAccess && !expression->children.empty() &&
        expression->children[0] != nullptr) {
      const Expr& receiver = *expression->children[0];
      bool static_receiver = false;
      if (receiver.kind == ExprKind::kIdentifier) {
        const ClassInfo* resolved = symbols_.Resolve(receiver.text, &klass);
        const bool shadowed = lint_types_.count(receiver.text) != 0;
        static_receiver = resolved != nullptr && !shadowed;
      }
      if (!static_receiver) {
        const ClassInfo* receiver_class = ReceiverClass(klass, receiver);
        if (receiver_class != nullptr) {
          const ClassInfo* owner = nullptr;
          if (const Member* member_ptr =
                  FindMemberInChain(symbols_, *receiver_class, expression->text, &owner)) {
            const bool method_like = member_ptr->kind != MemberKind::kVariable &&
                                     member_ptr->kind != MemberKind::kConstant;
            check_hidden(*member_ptr, owner, ExpressionStart(*expression), method_like);
            if (HasStaticAnnotation(*member_ptr)) {
              const std::string dedupe = std::to_string(expression->member_position.line) + ":" +
                                         std::to_string(expression->member_position.column) + ":" +
                                         member_ptr->name;
              if (reported_static.insert(dedupe).second) {
                LintStaticAccess(klass, expression->member_position, member_ptr->name);
              }
            }
          }
        }
      }
    }
    for (const ExprPtr& child : expression->children)
      check_static(child.get());
  };

  const auto has_operator_overload = [this](const ClassInfo& klass, const std::string& name) {
    const ClassInfo* cursor = &klass;
    std::set<const ClassInfo*> visited;
    while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
      for (const Member& candidate : cursor->declaration->members) {
        if (candidate.kind == MemberKind::kMethod && candidate.name == name)
          return true;
      }
      if (cursor->declaration->base_name.empty())
        break;
      cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
    }
    return false;
  };
  std::set<const Expr*> invalid_expressions;
  bool await_reported = false;
  std::function<void(const Expr*, bool)> check_special_expressions;
  check_special_expressions = [&](const Expr* expression, bool nested) {
    if (expression == nullptr)
      return;
    if (expression->kind == ExprKind::kCall && !expression->children.empty() &&
        expression->children[0] != nullptr) {
      const ClassInfo* owner = nullptr;
      const Member* target = ResolveCalledMember(klass, *expression->children[0], &owner);
      if (target != nullptr && nested && HasAnnotationName(target->annotations, "调试")) {
        error(ExpressionStart(*expression->children[0]),
              "方法\"" + target->name + "\"为调试方法，只能在最外层调用");
        invalid_expressions.insert(expression);
      }
    }
    for (const ExprPtr& child : expression->children) {
      check_special_expressions(child.get(), true);
    }
    if (expression->kind == ExprKind::kAwait) {
      if (!await_reported) {
        await_reported = true;
        error(member.name_position, "'等待'只能在异步方法方法体中使用");
        invalid_expressions.insert(expression);
      }
    }
  };
  std::function<void(const std::vector<StmtPtr>&)> check_special_statements;
  check_special_statements = [&](const std::vector<StmtPtr>& statements) {
    for (const StmtPtr& statement : statements) {
      if (statement == nullptr)
        continue;
      const bool statement_root = statement->kind == StmtKind::kExpression;
      check_special_expressions(statement->expr.get(), !statement_root);
      check_special_expressions(statement->loop_begin.get(), true);
      check_special_expressions(statement->loop_end.get(), true);
      check_special_expressions(statement->loop_step.get(), true);
      for (const SwitchCase& switch_case : statement->cases) {
        for (const ExprPtr& value : switch_case.values) {
          check_special_expressions(value.get(), true);
        }
        check_special_statements(switch_case.body);
      }
      check_special_statements(statement->body);
      check_special_statements(statement->else_body);
    }
  };
  check_special_statements(member.body);
  if (member.expression_body != nullptr) {
    check_special_expressions(member.expression_body.get(), true);
  }

  std::function<void(const Expr*)> walk_type_expression = [&](const Expr* expression) {
    if (expression == nullptr)
      return;
    for (const ExprPtr& child : expression->children)
      walk_type_expression(child.get());
    if (expression->kind == ExprKind::kBinary) {
      const std::string& op = expression->text;
      if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        const std::string left_type =
            expression->children.size() > 0 && expression->children[0] != nullptr
                ? InferTypeName(klass, *expression->children[0])
                : std::string();
        const std::string right_type =
            expression->children.size() > 1 && expression->children[1] != nullptr
                ? InferTypeName(klass, *expression->children[1])
                : std::string();
        const ClassInfo* left_class = symbols_.Resolve(left_type, &klass);
        const ClassInfo* right_class = symbols_.Resolve(right_type, &klass);
        if (left_class == nullptr || right_class == nullptr)
          return;
        const int left_rank = PrimitiveRank(PrimitiveNameOf(*left_class));
        const int right_rank = PrimitiveRank(PrimitiveNameOf(*right_class));
        if (left_rank >= 0 && right_rank >= 0)
          return;
        if (has_operator_overload(*left_class, op))
          return;
        const bool left_text =
            left_class->has_java_alias &&
            (left_class->java_alias == "java.lang.String" || left_class->java_alias == "String");
        const bool right_text =
            right_class->has_java_alias &&
            (right_class->java_alias == "java.lang.String" || right_class->java_alias == "String");
        if (left_text && !right_text)
          return;
        if (left_rank >= 0 && right_text) {
          invalid_expressions.insert(expression);
          error(ExpressionStart(*expression->children[0]),
                "二元表达式左值和右值必须是数值类型或重载了运算符\"" + op + "\"的类型");
          return;
        }
        if (left_text && right_text) {
          const bool left_literal = expression->children[0]->kind == ExprKind::kLiteral;
          const bool right_literal = expression->children[1]->kind == ExprKind::kLiteral;
          if (left_literal && right_literal) {
            invalid_expressions.insert(expression);
            error(ExpressionStart(*expression->children[0]),
                  "二元表达式左值和右值必须是数值类型或重载了运算符\"" + op + "\"的类型");
          }
          return;
        }
        return;
      }
      return;
    }
    if (expression->kind == ExprKind::kArrayAccess) {
      if (expression->children.size() < 2 || expression->children[0] == nullptr)
        return;
      const std::string target_type = InferTypeName(klass, *expression->children[0]);
      if (target_type.empty() || target_type.find('[') != std::string::npos)
        return;
      const ClassInfo* target_class = symbols_.Resolve(BaseResolvableName(target_type), &klass);
      if (target_class != nullptr && has_operator_overload(*target_class, kOperatorIndex))
        return;
      invalid_expressions.insert(expression);
      error(ExpressionStart(*expression), "访问的对象不是数组或重载了\"[]\"运算符的对象");
      return;
    }
    if (expression->kind != ExprKind::kCall || expression->children.empty() ||
        expression->children[0] == nullptr) {
      return;
    }
    const Expr& callee = *expression->children[0];
    const ClassInfo* owner = nullptr;
    const Member* target = ResolveCalledMember(klass, callee, &owner);
    if (target == nullptr || target->kind != MemberKind::kMethod)
      return;
    const std::size_t provided = expression->children.size() - 1;
    for (std::size_t i = 0; i < target->parameters.size() && i < provided; ++i) {
      const Expr* argument = expression->children[i + 1].get();
      if (argument == nullptr || target->parameters[i].type_name.empty())
        continue;
      const std::string source_type = InferTypeName(klass, *argument);
      if (invalid_expressions.count(argument) != 0 ||
          primitive_mismatch(target->parameters[i].type_name, source_type)) {
        error(ExpressionStart(*argument),
              "调用方法\"" + target->name + "\"的第" + std::to_string(i + 1) + "个参数类型不匹配");
      }
    }
  };

  const bool has_code_statement = ContainsCodeStatement(member.body);
  const bool flow_checked = member.code_text.empty() && !member.no_body &&
                            member.expression_body == nullptr && !member.body.empty();
  const bool return_checked = lint_returns_value_ && !lint_return_type_.empty() &&
                              member.code_text.empty() && !member.no_body &&
                              member.expression_body == nullptr && !has_code_statement;
  const bool void_method = member.kind == MemberKind::kMethod && member.type_name.empty() &&
                           member.code_text.empty() && !member.no_body &&
                           member.expression_body == nullptr;

  std::function<void(const std::vector<StmtPtr>&, bool)> walk_block;
  std::function<void(const Stmt&, bool)> walk_statement;

  std::vector<std::set<std::string>> scopes;

  const auto java_extends = [this](const ClassInfo* info) {
    const ClassInfo* cursor = info;
    std::set<const ClassInfo*> visited;
    while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
      if (cursor->has_java_alias)
        return true;
      if (cursor->declaration->base_name.empty())
        break;
      cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
    }
    return false;
  };
  const auto known_local_name = [&](const std::string& name) {
    for (const auto& scope : scopes) {
      if (scope.count(name) != 0)
        return true;
    }
    return false;
  };
  std::function<void(const Expr*, bool)> check_symbols;
  check_symbols = [&](const Expr* expression, bool as_callee) {
    if (expression == nullptr)
      return;
    switch (expression->kind) {
      case ExprKind::kLiteral:
      case ExprKind::kThis:
      case ExprKind::kSuper:
      case ExprKind::kNull:
      case ExprKind::kUnknown:
        return;
      case ExprKind::kIdentifier: {
        const std::string& name = expression->text;
        if (known_local_name(name))
          return;
        const ClassInfo* owner = nullptr;
        if (FindMemberInChain(symbols_, klass, name, &owner) != nullptr)
          return;
        if (FindGlobalClassMember(symbols_, klass, name, &owner) != nullptr)
          return;
        if (symbols_.Resolve(name, &klass) != nullptr)
          return;
        if (klass.has_java_alias)
          return;
        if (klass.declaration != nullptr) {
          for (const std::string& generic : klass.declaration->generic_parameters) {
            if (generic == name)
              return;
          }
        }
        error(ExpressionStart(*expression), "未知符号: \"" + name + "\"");
        invalid_expressions.insert(expression);
        return;
      }
      case ExprKind::kFieldAccess: {
        if (expression->children.empty() || expression->children[0] == nullptr)
          return;
        const Expr* receiver = expression->children[0].get();
        const std::string receiver_type = InferTypeName(klass, *receiver);
        if (!receiver_type.empty() && receiver_type.back() == ']') {
          if (expression->text != "长度") {
            error(expression->member_position, "数组只有\"长度\"属性");
            invalid_expressions.insert(expression);
            return;
          }
        }
        const ClassInfo* receiver_class = nullptr;
        bool receiver_is_class = false;
        if (receiver->kind == ExprKind::kIdentifier && !known_local_name(receiver->text)) {
          receiver_class = symbols_.Resolve(receiver->text, &klass);
          if (receiver_class != nullptr)
            receiver_is_class = true;
        }
        if (!receiver_is_class) {
          receiver_class = LintReceiverClass(klass, *receiver);
        }
        if (receiver_class == nullptr) {
          if (receiver->kind == ExprKind::kIdentifier) {
            check_symbols(receiver, false);
            if (invalid_expressions.count(receiver) != 0) {
              error(expression->member_position, "\"" + expression->text + "\"不可访问");
              invalid_expressions.insert(expression);
            }
          }
          return;
        }
        if (java_extends(receiver_class)) {
          if (!receiver_is_class)
            check_symbols(receiver, false);
          return;
        }
        const ClassInfo* owner = nullptr;
        const Member* member =
            FindMemberInChain(symbols_, *receiver_class, expression->text, &owner);
        if (member == nullptr) {
          if (as_callee) {
            error(expression->member_position, "找不到方法: \"" + expression->text + "\"");
          } else {
            error(expression->member_position,
                  "找不到变量/常量/属性读方法: \"" + expression->text + "\"");
          }
          invalid_expressions.insert(expression);
          return;
        }
        if (!receiver_is_class)
          check_symbols(receiver, false);
        return;
      }
      case ExprKind::kCall: {
        if (expression->children.empty() || expression->children[0] == nullptr)
          return;
        const Expr* callee = expression->children[0].get();
        bool callee_valid = true;
        if (callee->kind == ExprKind::kIdentifier) {
          const ClassInfo* owner = nullptr;
          const Member* resolved = ResolveCalledMember(klass, *callee, &owner);
          if (resolved == nullptr) {
            if (!klass.has_java_alias) {
              error(ExpressionStart(*callee), "找不到方法: \"" + callee->text + "\"");
            }
            callee_valid = false;
          }
        } else if (callee->kind == ExprKind::kFieldAccess) {
          check_symbols(callee, true);
          callee_valid = invalid_expressions.count(callee) == 0;
        } else {
          check_symbols(callee, false);
          callee_valid = invalid_expressions.count(callee) == 0;
        }
        for (std::size_t i = 1; i < expression->children.size(); ++i) {
          if (expression->children[i] != nullptr) {
            check_symbols(expression->children[i].get(), false);
          }
        }
        if (!callee_valid)
          invalid_expressions.insert(expression);
        return;
      }
      case ExprKind::kNew:
      case ExprKind::kCast:
      case ExprKind::kIsType: {
        std::string type_text = expression->text;
        while (!type_text.empty() && type_text.back() == '?')
          type_text.pop_back();
        while (EndsWith(type_text, kOperatorIndex)) {
          type_text = type_text.substr(0, type_text.size() - 2);
        }
        if (type_text.find('<') == std::string::npos && !type_text.empty() &&
            symbols_.Resolve(type_text, &klass) == nullptr) {
          error(expression->type_position, "找不到类\"" + type_text + "\"");
          invalid_expressions.insert(expression);
          if (expression->kind == ExprKind::kCast && !expression->children.empty() &&
              expression->children[0] != nullptr) {
            std::string source_type = InferTypeName(klass, *expression->children[0]);
            if (source_type.empty())
              source_type = "无类型";
            error(ExpressionStart(*expression->children[0]),
                  "类型\"" + source_type + "\"无法强制转换为类型\"无类型\"");
          }
        }
        for (const ExprPtr& child : expression->children) {
          check_symbols(child.get(), false);
        }
        return;
      }
      default:
        for (const ExprPtr& child : expression->children) {
          check_symbols(child.get(), false);
        }
        return;
    }
  };

  walk_statement = [&](const Stmt& statement, bool reachable) {
    switch (statement.kind) {
      case StmtKind::kVariable: {
        if (!statement.type_name.empty()) {
          check_type(statement.type_name, statement.type_position);
        }
        const bool duplicate = scopes.back().count(statement.name) != 0;
        if (duplicate) {
          error(statement.position, "变量\"" + statement.name + "\"在当前作用域下重复定义");
        } else {
          bool shadows = false;
          for (std::size_t i = 0; i + 1 < scopes.size(); ++i) {
            if (scopes[i].count(statement.name) != 0)
              shadows = true;
          }
          if (shadows) {
            warn(statement.position,
                 "变量\"" + statement.name + "\"在外层作用域有定义，当前定义会遮蔽外层的变量定义",
                 "var.shadowed");
          }
        }
        scopes.back().insert(statement.name);
        if (!statement.type_name.empty())
          lint_types_[statement.name] = statement.type_name;
        check_symbols(statement.expr.get(), false);
        check_static(statement.expr.get());
        walk_type_expression(statement.expr.get());
        if (!statement.type_name.empty() && statement.expr != nullptr) {
          LintAutoCast(klass, statement.expr->position, statement.type_name, statement.expr.get());
          if (statement.expr->kind != ExprKind::kMulti) {
            const std::string source_type = InferTypeName(klass, *statement.expr);
            bool same_unresolved_creation = false;
            if (statement.expr->kind == ExprKind::kNew &&
                BaseResolvableName(statement.expr->text) ==
                    BaseResolvableName(statement.type_name)) {
              same_unresolved_creation = true;
            }
            if (!same_unresolved_creation &&
                (invalid_expressions.count(statement.expr.get()) != 0 ||
                 primitive_mismatch(statement.type_name, source_type))) {
              error(statement.type_position,
                    "变量\"" + statement.name + "\"初始值和声明类型不匹配");
            }
          }
        }
        break;
      }
      case StmtKind::kReturn: {
        if (statement.expr != nullptr) {
          check_symbols(statement.expr.get(), false);
          check_static(statement.expr.get());
          walk_type_expression(statement.expr.get());
          if (statement.expr->kind == ExprKind::kMulti && !void_method) {
            error(ExpressionStart(*statement.expr), "返回值类型与方法类型不匹配");
          }
          if (void_method) {
            error(statement.expr->position, "返回值类型与方法类型不匹配");
          } else if (!lint_return_type_.empty()) {
            LintAutoCast(klass, statement.expr->position, lint_return_type_, statement.expr.get());
            const std::string source_type = InferTypeName(klass, *statement.expr);
            if (invalid_expressions.count(statement.expr.get()) != 0 ||
                primitive_mismatch(lint_return_type_, source_type)) {
              error(ExpressionStart(*statement.expr), "返回值类型与方法类型不匹配");
            }
          }
        }
        break;
      }
      case StmtKind::kExpression: {
        const Expr* expression = statement.expr.get();
        check_symbols(expression, false);
        check_static(expression);
        walk_type_expression(expression);
        if (expression != nullptr && expression->kind == ExprKind::kExchange &&
            expression->children.size() == 2 && expression->children[0] != nullptr &&
            expression->children[1] != nullptr) {
          const bool left_index = expression->children[0]->kind == ExprKind::kArrayAccess;
          const bool right_index = expression->children[1]->kind == ExprKind::kArrayAccess;
          if (left_index != right_index) {
            error(ExpressionStart(*expression->children[0]), "交换式左右值格式不同");
          } else {
            for (const ExprPtr& operand : expression->children) {
              if (operand == nullptr || operand->kind != ExprKind::kArrayAccess ||
                  operand->children.empty() || operand->children[0] == nullptr) {
                continue;
              }
              const std::string receiver_type = InferTypeName(klass, *operand->children[0]);
              if (receiver_type.empty() || receiver_type.find('[') != std::string::npos)
                continue;
              const ClassInfo* receiver =
                  symbols_.Resolve(BaseResolvableName(receiver_type), &klass);
              const ClassInfo* owner = nullptr;
              if (receiver != nullptr &&
                  FindMemberInChain(symbols_, *receiver, kOperatorIndexAssign, &owner) == nullptr) {
                error(ExpressionStart(*operand),
                      "访问的对象未重载\"[]=\"运算符，不可赋值或进行交换");
                break;
              }
            }
          }
        }
        if (expression != nullptr && expression->kind == ExprKind::kAssign &&
            expression->children.size() == 2 && expression->children[0] != nullptr &&
            expression->children[1] != nullptr) {
          const std::string target_type = InferTypeName(klass, *expression->children[0]);
          LintAutoCast(klass, expression->children[1]->position, target_type,
                       expression->children[1].get());
          const std::string source_type = InferTypeName(klass, *expression->children[1]);
          const bool lvalue_invalid = expression->children[0]->kind == ExprKind::kArrayAccess &&
                                      invalid_expressions.count(expression->children[0].get()) != 0;
          if (lvalue_invalid) {
            error(ExpressionStart(*expression->children[1]), "右值类型不匹配左值类型");
          } else if (expression->children[0]->kind == ExprKind::kArrayAccess) {
            const Expr& access = *expression->children[0];
            const std::string receiver_type = InferTypeName(klass, *access.children[0]);
            const bool receiver_is_array =
                !receiver_type.empty() && receiver_type.find('[') != std::string::npos;
            const ClassInfo* receiver =
                !receiver_is_array && !receiver_type.empty()
                    ? symbols_.Resolve(BaseResolvableName(receiver_type), &klass)
                    : nullptr;
            const ClassInfo* owner = nullptr;
            const Member* writer =
                receiver != nullptr
                    ? FindMemberInChain(symbols_, *receiver, kOperatorIndexAssign, &owner)
                    : nullptr;
            if (receiver_is_array) {
              if (!target_type.empty() &&
                  (invalid_expressions.count(expression->children[1].get()) != 0 ||
                   primitive_mismatch(target_type, source_type))) {
                error(ExpressionStart(*expression->children[1]), "右值类型不匹配左值类型");
              }
            } else if (receiver != nullptr && writer == nullptr) {
              error(ExpressionStart(access), "访问的对象未重载\"[]=\"运算符，不可赋值或进行交换");
              error(ExpressionStart(*expression->children[1]),
                    "右值类型不匹配运算符重载方法\"[]=\"的第2个参数类型");
            } else if (writer != nullptr && writer->parameters.size() >= 2 &&
                       !writer->parameters[1].type_name.empty()) {
              if (invalid_expressions.count(expression->children[1].get()) != 0 ||
                  primitive_mismatch(writer->parameters[1].type_name, source_type)) {
                error(ExpressionStart(*expression->children[1]),
                      "右值类型不匹配运算符重载方法\"[]=\"的第2个参数类型");
              }
            }
          } else if (!target_type.empty() &&
                     (invalid_expressions.count(expression->children[1].get()) != 0 ||
                      primitive_mismatch(target_type, source_type))) {
            error(ExpressionStart(*expression->children[1]), "右值类型不匹配左值类型");
          }
        }
        break;
      }
      case StmtKind::kIf: {
        check_symbols(statement.expr.get(), false);
        check_static(statement.expr.get());
        walk_type_expression(statement.expr.get());
        scopes.emplace_back();
        walk_block(statement.body, true);
        scopes.pop_back();
        if (!statement.else_body.empty()) {
          scopes.emplace_back();
          walk_block(statement.else_body, true);
          scopes.pop_back();
        }
        break;
      }
      case StmtKind::kLoopCount: {
        check_symbols(statement.loop_begin.get(), false);
        check_symbols(statement.loop_end.get(), false);
        check_symbols(statement.loop_step.get(), false);
        check_static(statement.loop_begin.get());
        check_static(statement.loop_end.get());
        check_static(statement.loop_step.get());
        walk_type_expression(statement.loop_begin.get());
        walk_type_expression(statement.loop_end.get());
        walk_type_expression(statement.loop_step.get());
        scopes.emplace_back();
        if (!statement.loop_variable.empty())
          scopes.back().insert(statement.loop_variable);
        walk_block(statement.body, true);
        scopes.pop_back();
        break;
      }
      case StmtKind::kLoopWhile:
      case StmtKind::kLoopForeach: {
        check_symbols(statement.expr.get(), false);
        check_symbols(statement.loop_begin.get(), false);
        check_symbols(statement.loop_end.get(), false);
        check_static(statement.expr.get());
        check_static(statement.loop_begin.get());
        check_static(statement.loop_end.get());
        walk_type_expression(statement.expr.get());
        walk_type_expression(statement.loop_begin.get());
        walk_type_expression(statement.loop_end.get());
        scopes.emplace_back();
        if (!statement.loop_variable.empty())
          scopes.back().insert(statement.loop_variable);
        if (!statement.loop_key_name.empty())
          scopes.back().insert(statement.loop_key_name);
        if (!statement.loop_value_name.empty())
          scopes.back().insert(statement.loop_value_name);
        walk_block(statement.body, true);
        scopes.pop_back();
        break;
      }
      case StmtKind::kBlock: {
        scopes.emplace_back();
        walk_block(statement.body, true);
        scopes.pop_back();
        break;
      }
      case StmtKind::kSwitch: {
        check_static(statement.expr.get());
        for (const SwitchCase& switch_case : statement.cases) {
          for (const ExprPtr& value : switch_case.values)
            check_static(value.get());
          scopes.emplace_back();
          walk_block(switch_case.body, true);
          scopes.pop_back();
        }
        break;
      }
      default:
        check_static(statement.expr.get());
        break;
    }
    (void)reachable;
  };

  walk_block = [&](const std::vector<StmtPtr>& statements, bool reachable) {
    bool current = reachable;
    for (const StmtPtr& statement : statements) {
      if (statement == nullptr)
        continue;
      if (!current) {
        if (flow_checked) {
          warn(statement->position, "前面已有返回值，当前语句不可达", "method.return");
        }
        continue;
      }
      walk_statement(*statement, current);
      if (flow_checked && StatementAlwaysReturns(*statement))
        current = false;
    }
  };

  scopes.emplace_back();
  for (const Parameter& parameter : member.parameters)
    scopes.back().insert(parameter.name);
  if (member.expression_body != nullptr) {
    check_symbols(member.expression_body.get(), false);
    check_static(member.expression_body.get());
    if (!lint_return_type_.empty()) {
      LintAutoCast(klass, member.expression_body->position, lint_return_type_,
                   member.expression_body.get());
    }
  } else {
    walk_block(member.body, true);
  }
  scopes.pop_back();

  if (return_checked && !ListAlwaysReturns(member.body)) {
    warn(member.name_position, "方法中并不是所有分支都返回了值", "method.return");
  }
  lint_types_.clear();
  lint_parameters_.clear();
  lint_return_type_.clear();
}

void JavaEmitter::RemoveUnusedLocalsAfterLint() {
  if (diagnostics_.has_errors())
    return;
  std::set<const ClassDecl*> visited;
  for (const std::string& class_name : symbols_.class_names()) {
    const auto found = symbols_.classes().find(class_name);
    if (found == symbols_.classes().end() || found->second.declaration == nullptr)
      continue;
    const ClassDecl* declaration = found->second.declaration;
    if (!visited.insert(declaration).second)
      continue;
    tiecodec::RemoveUnusedLocals(const_cast<ClassDecl*>(declaration), declaration->file,
                                 &diagnostics_);
  }
}

}
