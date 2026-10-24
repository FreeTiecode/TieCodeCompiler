#include "constants.h"
#include "java_emitter.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>

#include "internal/helpers.h"
#include "local_use.h"
#include "symbol_resolution.h"

namespace tiecodec {

using namespace detail;

namespace {

bool MayBeStringOperand(const Expr& expression) {
  if (expression.kind == ExprKind::kLiteral) {
    return !expression.raw.empty() && (expression.raw[0] == '"' || expression.raw[0] == '\'');
  }
  if (expression.kind == ExprKind::kBinary) {
    for (const ExprPtr& child : expression.children) {
      if (child != nullptr && MayBeStringOperand(*child))
        return true;
    }
    return false;
  }
  return true;
}

}

void JavaEmitter::CollectFromExpression(const ClassInfo& klass, const Expr* expression,
                                        std::set<std::string>* imports) {
  if (expression == nullptr)
    return;
  if (expression->kind == ExprKind::kBinary && expression->text == "+") {
    std::string folded;
    if (MayBeStringOperand(*expression) && FoldStringConcat(*expression, &folded)) {
      if (const ClassInfo* text = symbols_.Resolve(builtin_type::kText, &klass)) {
        if (text != &klass)
          imports->insert(names_.ImportPath(*text));
      }
    }
  }
  switch (expression->kind) {
    case ExprKind::kNew:
    case ExprKind::kCast:
    case ExprKind::kIsType: {
      const std::string type_text = SubstituteGenericType(expression->text);
      if (type_text.find('<') != std::string::npos) {
        CollectGenericTypeImports(klass, type_text, imports, false);
        break;
      }
      const ClassInfo* target = symbols_.Resolve(type_text, &klass);
      if (target != nullptr && target != &klass)
        imports->insert(names_.ImportPath(*target));
      break;
    }
    case ExprKind::kCall: {
      if (!expression->children.empty() && expression->children[0] != nullptr) {
        const ClassInfo* owner = nullptr;
        const Member* member = ResolveCalledMember(klass, *expression->children[0], &owner);
        if (member != nullptr && owner != nullptr && owner != &klass) {
          imports->insert(names_.ImportPath(*owner));
        }
      }
      break;
    }
    default:
      break;
  }
  for (const ExprPtr& child : expression->children) {
    CollectFromExpression(klass, child.get(), imports);
  }
}

void JavaEmitter::CollectCalleeOwners(const ClassInfo& klass, const Expr* expression,
                                      std::set<std::string>* imports) {
  if (expression == nullptr)
    return;
  switch (expression->kind) {
    case ExprKind::kCall: {
      if (!expression->children.empty() && expression->children[0] != nullptr) {
        const ClassInfo* owner = nullptr;
        const Member* member = ResolveCalledMember(klass, *expression->children[0], &owner);
        if (member != nullptr && owner != nullptr) {
          const Expr* receiver = nullptr;
          if (expression->children[0]->kind == ExprKind::kFieldAccess &&
              !expression->children[0]->children.empty()) {
            receiver = expression->children[0]->children[0].get();
          }
          if (receiver != nullptr) {
            CollectMemberAccessImports(klass, owner, ReceiverTypeText(klass, receiver), imports);
          } else if (owner != &klass) {
            imports->insert(names_.ImportPath(*owner));
          }
        }
      }
      break;
    }
    case ExprKind::kFieldAccess: {
      if (!expression->children.empty() && expression->children[0] != nullptr) {
        const std::string receiver_text = ReceiverTypeText(klass, expression->children[0].get());
        const ClassInfo* receiver_class = ReceiverClass(klass, *expression->children[0]);
        if (receiver_class == nullptr && !receiver_text.empty()) {
          std::string base_name;
          std::vector<std::string> arguments;
          if (SplitGenericType(receiver_text, &base_name, &arguments)) {
            receiver_class = symbols_.Resolve(base_name, &klass);
          } else {
            receiver_class = symbols_.Resolve(receiver_text, &klass);
          }
        }
        if (receiver_class != nullptr) {
          const ClassInfo* owner = nullptr;
          const Member* member =
              FindMemberInChain(symbols_, *receiver_class, expression->text, &owner);
          if (member != nullptr && owner != nullptr) {
            CollectMemberAccessImports(klass, owner, receiver_text, imports);
          }
        }
      }
      break;
    }
    case ExprKind::kAssign: {
      if (expression->children.size() == 2 && expression->children[0] != nullptr) {
        const Expr* target = expression->children[0].get();
        if (target->kind == ExprKind::kFieldAccess && !target->children.empty() &&
            target->children[0] != nullptr) {
          const ClassInfo* receiver_class = ReceiverClass(klass, *target->children[0]);
          if (receiver_class != nullptr) {
            const ClassInfo* owner = nullptr;
            const Member* member =
                FindMemberInChain(symbols_, *receiver_class, target->text, &owner);
            if (member != nullptr && owner != nullptr && owner != &klass) {
              imports->insert(names_.ImportPath(*owner));
            }
          }
        }
      }
      break;
    }
    default:
      break;
  }
  for (const ExprPtr& child : expression->children) {
    CollectCalleeOwners(klass, child.get(), imports);
  }
}

void JavaEmitter::CollectStatementImports(const ClassInfo& klass, const Stmt* statement,
                                          std::set<std::string>* imports) {
  if (statement == nullptr)
    return;
  if (!statement->type_name.empty()) {
    const std::string type_text = SubstituteGenericType(statement->type_name);
    if (type_text.find('<') != std::string::npos) {
      CollectGenericTypeImports(klass, type_text, imports, false);
    } else if (const ClassInfo* type = symbols_.Resolve(type_text, &klass)) {
      if (type != &klass)
        imports->insert(names_.ImportPath(*type));
    }
  }
  if (statement->kind == StmtKind::kLoopCount) {
    if (const ClassInfo* counter_type = symbols_.Resolve(builtin_type::kInteger, &klass)) {
      imports->insert(names_.ImportPath(*counter_type));
    }
  }
  if (statement->kind == StmtKind::kVariable && !statement->name.empty() &&
      !statement->type_name.empty()) {
    import_local_types_[statement->name] = statement->type_name;
  }
  if (statement->kind == StmtKind::kEventBlock && !statement->event_object.empty() &&
      !statement->event_name.empty()) {
    for (const Parameter& parameter : statement->event_parameters) {
      if (!parameter.type_name.empty())
        import_local_types_[parameter.name] = parameter.type_name;
    }

    std::string receiver_type;
    const auto local = import_local_types_.find(statement->event_object);
    if (local != import_local_types_.end())
      receiver_type = local->second;
    if (receiver_type.empty())
      receiver_type = MemberTypeOf(klass, statement->event_object);
    const ClassInfo* receiver_class = symbols_.Resolve(receiver_type, &klass);
    if (receiver_class != nullptr) {
      const ClassInfo* owner = nullptr;
      if (const Member* definition =
              FindMemberInChain(symbols_, *receiver_class, statement->event_name, &owner)) {
        if (owner != nullptr && owner != &klass)
          imports->insert(names_.ImportPath(*owner));
        if (definition != nullptr && !definition->type_name.empty() && owner != nullptr) {
          if (const ClassInfo* type = symbols_.Resolve(definition->type_name, owner)) {
            if (type != &klass)
              imports->insert(names_.ImportPath(*type));
          }
        }
        if (definition != nullptr && owner != nullptr) {
          for (const Parameter& parameter : definition->parameters) {
            if (parameter.type_name.empty())
              continue;
            if (const ClassInfo* type = symbols_.Resolve(parameter.type_name, owner)) {
              if (type != &klass)
                imports->insert(names_.ImportPath(*type));
            }
          }
        }
      }
    }
  }
  CollectCalleeOwners(klass, statement->expr.get(), imports);
  CollectCalleeOwners(klass, statement->loop_begin.get(), imports);
  CollectCalleeOwners(klass, statement->loop_end.get(), imports);
  CollectCalleeOwners(klass, statement->loop_step.get(), imports);
  for (const SwitchCase& switch_case : statement->cases) {
    for (const ExprPtr& value : switch_case.values) {
      CollectCalleeOwners(klass, value.get(), imports);
    }
    for (const StmtPtr& child : switch_case.body) {
      CollectStatementImports(klass, child.get(), imports);
    }
  }
  for (const StmtPtr& child : statement->body) {
    CollectStatementImports(klass, child.get(), imports);
  }
  for (const StmtPtr& child : statement->else_body) {
    CollectStatementImports(klass, child.get(), imports);
  }
}

void JavaEmitter::CollectClassLiteralImports(const ClassInfo& klass, const Expr* expression,
                                             std::set<std::string>* imports) {
  if (expression == nullptr)
    return;
  if (expression->kind == ExprKind::kIdentifier && local_types_.count(expression->text) == 0 &&
      current_parameters_.count(expression->text) == 0) {
    const ClassInfo* member_owner = nullptr;
    const ClassInfo* global_owner = nullptr;
    if (FindMemberInChain(symbols_, klass, expression->text, &member_owner) != nullptr) {
    } else if (FindGlobalClassMember(symbols_, klass, expression->text, &global_owner) != nullptr) {
      if (global_owner != &klass)
        imports->insert(names_.ImportPath(*global_owner));
    } else if (const ClassInfo* target = symbols_.Resolve(expression->text, &klass)) {
      if (target != &klass)
        imports->insert(names_.ImportPath(*target));
    }
  }
  for (const ExprPtr& child : expression->children) {
    CollectClassLiteralImports(klass, child.get(), imports);
  }
}

void JavaEmitter::CollectImports(const ClassInfo& klass, std::set<std::string>* imports) {
  if (klass.declaration != nullptr) {
    const ClassInfo* scheduler = nullptr;
    for (const Member& member : klass.declaration->members) {
      if (!IsAsyncMember(member))
        continue;
      if (used_members_ != nullptr && used_members_->count(&member) == 0)
        continue;
      imports->insert(names_.Package(klass) + "." + AsyncContextTypeName(klass, member));
      if (!AsyncMemberUsesGlobalScheduler(member))
        continue;
      if (scheduler == nullptr)
        scheduler = symbols_.Resolve("异步调度器", &klass);
      if (scheduler != nullptr && scheduler != &klass)
        imports->insert(names_.ImportPath(*scheduler));
    }
  }
  import_local_types_.clear();
  local_types_.clear();
  current_parameters_.clear();
  current_parameter_types_.clear();
  renames_.clear();
  const auto add_java_imports = [imports](const std::vector<Annotation>& annotations) {
    for (const Annotation& annotation : annotations) {
      if (annotation.name != "导入Java")
        continue;
      const std::string value = annotation.TextArgument(kArgumentParameter);
      if (!value.empty())
        imports->insert(value);
    }
  };

  if (klass.declaration != nullptr && !klass.declaration->base_name.empty()) {
    const std::string base_name_text = SubstituteGenericType(klass.declaration->base_name);
    if (base_name_text.find('<') != std::string::npos) {
      CollectGenericTypeImports(klass, base_name_text, imports, true);
    } else {
      const ClassInfo* base_class = symbols_.Resolve(base_name_text, &klass);
      if (base_class != nullptr)
        imports->insert(names_.ImportPath(*base_class));
    }
  }
  if (klass.has_java_alias) {
    if (klass.declaration != nullptr)
      add_java_imports(klass.declaration->annotations);
    {
      imports->insert(config_.Option(option::kRNamespace, config_.namespace_name) + ".R");
    }
  }

  if (klass.declaration != nullptr && IsComponentContainerClass(klass) &&
      !CollectComponentFields(klass).empty()) {
    const std::string type_name = OnCreateComponentReturnTypeName(klass);
    const ClassInfo* cursor = symbols_.Resolve(type_name, &klass);
    std::set<const ClassInfo*> visited;
    while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
      if (!cursor->output_name.empty() && cursor != &klass) {
        imports->insert(names_.ImportPath(*cursor));
      }
      if (cursor->declaration->base_name.empty())
        break;
      cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
    }
  }
  const auto add_annotation_imports = [this, &klass,
                                       imports](const std::vector<Annotation>& annotations) {
    for (const Annotation& annotation : annotations) {
      for (const AnnotationArgument& argument : annotation.arguments) {
        const std::size_t dot = argument.value.rfind('.');
        if (dot == std::string::npos)
          continue;
        if (const ClassInfo* referenced = symbols_.Resolve(argument.value.substr(0, dot), &klass)) {
          if (referenced != &klass)
            imports->insert(names_.ImportPath(*referenced));
        }
      }
    }
  };
  if (klass.declaration != nullptr) {
    add_java_imports(klass.declaration->annotations);
    add_annotation_imports(klass.declaration->annotations);
  }

  if (klass.declaration != nullptr) {
    for (const Member& member : klass.declaration->members) {
      if (used_members_ != nullptr && used_members_->count(&member) == 0)
        continue;
      add_java_imports(member.annotations);
      add_annotation_imports(member.annotations);
      CollectCalleeOwners(klass, member.expression_body.get(), imports);

      if (member.kind == MemberKind::kEventBlock && !member.event_object.empty() &&
          !member.name.empty()) {
        std::string type_name = MemberTypeOf(klass, member.event_object);
        const auto local = local_types_.find(member.event_object);
        if (type_name.empty() && local != local_types_.end())
          type_name = local->second;
        const ClassInfo* receiver_class = type_name.empty()
                                              ? symbols_.Resolve(member.event_object, &klass)
                                              : symbols_.Resolve(type_name, &klass);
        if (receiver_class != nullptr) {
          const ClassInfo* event_owner = nullptr;
          if (const Member* definition =
                  FindMemberInChain(symbols_, *receiver_class, member.name, &event_owner)) {
            if (definition->kind == MemberKind::kEventDefinition && event_owner != &klass) {
              imports->insert(names_.ImportPath(*event_owner));
            }
          }
        }
      }
      if (!member.code_text.empty())
        CollectCodeRefImports(klass, member.code_text, imports);
      const std::set<std::string> used_locals = UsedLocalNames(member);
      for (const StmtPtr& statement : member.body) {
        if (!IsLocalDeclarationUsed(*statement, used_locals))
          continue;
        if (statement != nullptr && statement->kind == StmtKind::kCode) {
          CollectCodeRefImports(klass, statement->code_text, imports);
        }
      }
      if (resources_ != nullptr && resources_->IsResourceField(klass, member)) {
        imports->insert(resources_->ResourceImport(klass));
      }
      if (!member.type_name.empty()) {
        const std::string type_text = SubstituteGenericType(member.type_name);
        if (type_text.find('<') != std::string::npos) {
          CollectGenericTypeImports(klass, type_text, imports, false);
        } else {
          const ClassInfo* type = symbols_.Resolve(type_text, &klass);
          if (type != nullptr && type != &klass)
            imports->insert(names_.ImportPath(*type));
        }
      }
      for (const Parameter& parameter : member.parameters) {
        if (parameter.type_name.empty())
          continue;
        const std::string type_text = SubstituteGenericType(parameter.type_name);
        if (type_text.find('<') != std::string::npos) {
          CollectGenericTypeImports(klass, type_text, imports, false);
          continue;
        }
        const ClassInfo* type = symbols_.Resolve(type_text, &klass);
        if (type != nullptr && type != &klass)
          imports->insert(names_.ImportPath(*type));
      }
      for (const StmtPtr& statement : member.body) {
        if (!IsLocalDeclarationUsed(*statement, used_locals))
          continue;
        CollectStatementImports(klass, statement.get(), imports);
        CollectClassLiteralImports(klass, statement->expr.get(), imports);
        CollectFromExpression(klass, statement->expr.get(), imports);
        for (const StmtPtr& child : statement->body) {
          CollectFromExpression(klass, child->expr.get(), imports);
        }
        for (const StmtPtr& child : statement->else_body) {
          CollectFromExpression(klass, child->expr.get(), imports);
        }
      }
      CollectFromExpression(klass, member.expression_body.get(), imports);
    }
  }
  {
    imports->insert(config_.Option(option::kRNamespace, config_.namespace_name) + ".R");
  }
}

std::string JavaEmitter::SubstituteTypeParameters(const std::string& type_name,
                                                  const std::vector<std::string>& parameters,
                                                  const std::vector<std::string>& arguments) {
  std::string result = type_name;
  std::size_t replace_count = std::min(parameters.size(), arguments.size());
  for (std::size_t i = 0; i < replace_count; ++i) {
    const std::string& from = parameters[i];
    if (from.empty())
      continue;
    const std::string& to = arguments[i];
    std::string replaced;
    replaced.reserve(result.size());
    std::size_t index = 0;
    while (index < result.size()) {
      if (result.compare(index, from.size(), from) == 0) {
        const bool left_ok = index == 0 || !IsIdentifierByte(result[index - 1]);
        const std::size_t after = index + from.size();
        const bool right_ok = after >= result.size() || !IsIdentifierByte(result[after]);
        if (left_ok && right_ok) {
          replaced += to;
          index = after;
          continue;
        }
      }
      replaced.push_back(result[index]);
      ++index;
    }
    result = std::move(replaced);
  }
  return result;
}

std::string JavaEmitter::ReceiverTypeText(const ClassInfo& klass, const Expr* receiver) {
  if (receiver == nullptr)
    return {};
  switch (receiver->kind) {
    case ExprKind::kIdentifier: {
      const auto local = import_local_types_.find(receiver->text);
      if (local != import_local_types_.end())
        return local->second;
      const std::string member_type = MemberTypeOf(klass, receiver->text);
      if (!member_type.empty())
        return member_type;
      if (symbols_.Resolve(receiver->text, &klass) != nullptr)
        return receiver->text;
      return {};
    }
    case ExprKind::kParen:
      return receiver->children.empty() ? std::string()
                                        : ReceiverTypeText(klass, receiver->children[0].get());
    case ExprKind::kArrayAccess:
      if (receiver->children.empty() || receiver->children[0] == nullptr)
        return {};
      {
        std::string text = ReceiverTypeText(klass, receiver->children[0].get());
        while (EndsWith(text, kOperatorIndex)) {
          text = text.substr(0, text.size() - 2);
        }
        if (text.empty())
          return {};
        const ClassInfo* receiver_class = symbols_.Resolve(BaseResolvableName(text), &klass);
        if (receiver_class != nullptr) {
          const ClassInfo* owner = nullptr;
          if (const Member* indexer = FindMemberInChain(symbols_, *receiver_class, "[]", &owner)) {
            return indexer->type_name;
          }
        }
        return text;
      }
    case ExprKind::kNew:
      return receiver->text;
    case ExprKind::kFieldAccess:
    case ExprKind::kCall: {
      const Expr* callee = receiver;
      const Expr* base_expression = nullptr;
      if (receiver->kind == ExprKind::kFieldAccess && !receiver->children.empty()) {
        base_expression = receiver->children[0].get();
      } else if (receiver->kind == ExprKind::kCall && !receiver->children.empty()) {
        const Expr* call_callee = receiver->children[0].get();
        if (call_callee == nullptr)
          return {};
        if (call_callee->kind == ExprKind::kFieldAccess && !call_callee->children.empty()) {
          callee = call_callee;
          base_expression = call_callee->children[0].get();
        } else if (call_callee->kind == ExprKind::kIdentifier) {
          const ClassInfo* owner = nullptr;
          const Member* member = ResolveCalledMember(klass, *call_callee, &owner);
          return member != nullptr ? member->type_name : std::string();
        }
      }
      if (base_expression == nullptr)
        return {};
      const std::string receiver_text = ReceiverTypeText(klass, base_expression);
      if (receiver_text.empty())
        return {};
      std::string base_name;
      std::vector<std::string> arguments;
      std::vector<std::string> parameters;
      const ClassInfo* base_class = nullptr;
      if (SplitGenericType(receiver_text, &base_name, &arguments)) {
        base_class = symbols_.Resolve(base_name, &klass);
        if (base_class != nullptr && base_class->declaration != nullptr) {
          parameters = base_class->declaration->generic_parameters;
        }
      } else {
        base_class = symbols_.Resolve(receiver_text, &klass);
      }
      if (base_class == nullptr)
        return {};
      const ClassInfo* owner = nullptr;
      std::string member_name;
      if (callee->kind == ExprKind::kFieldAccess) {
        member_name = callee->text;
      } else {
        member_name = callee->text;
      }
      const Member* member = FindMemberInChain(symbols_, *base_class, member_name, &owner);
      if (member == nullptr)
        return {};
      if (owner != base_class) {
        const ClassInfo* cursor = base_class;
        std::vector<std::string> cursor_arguments = arguments;
        std::set<const ClassInfo*> visited;
        while (cursor != nullptr && cursor != owner && cursor->declaration != nullptr &&
               visited.insert(cursor).second) {
          const std::string base_text =
              SubstituteTypeParameters(cursor->declaration->base_name,
                                       cursor->declaration->generic_parameters, cursor_arguments);
          std::string base_base_name;
          std::vector<std::string> base_arguments;
          if (SplitGenericType(base_text, &base_base_name, &base_arguments)) {
            cursor = symbols_.Resolve(base_base_name, &klass);
            cursor_arguments = base_arguments;
          } else {
            cursor = symbols_.Resolve(base_text, &klass);
            cursor_arguments.clear();
          }
        }
        if (cursor != owner)
          return member->type_name;
        return SubstituteTypeParameters(member->type_name, owner->declaration->generic_parameters,
                                        cursor_arguments);
      }
      return SubstituteTypeParameters(member->type_name, parameters, arguments);
    }
    default:
      return {};
  }
}

void JavaEmitter::CollectMemberAccessImports(const ClassInfo& klass, const ClassInfo* owner,
                                             const std::string& receiver_type_text,
                                             std::set<std::string>* imports) {
  if (owner == nullptr || owner == &klass)
    return;
  if (receiver_type_text.find('<') == std::string::npos) {
    imports->insert(names_.ImportPath(*owner));
    return;
  }
  std::string base_name;
  std::vector<std::string> arguments;
  if (!SplitGenericType(receiver_type_text, &base_name, &arguments)) {
    imports->insert(names_.ImportPath(*owner));
    return;
  }
  const ClassInfo* cursor = symbols_.Resolve(base_name, &klass);
  std::vector<std::string> cursor_arguments = arguments;
  std::set<const ClassInfo*> visited;
  while (cursor != nullptr && cursor != owner && cursor->declaration != nullptr &&
         visited.insert(cursor).second) {
    const std::string base_text = SubstituteTypeParameters(
        cursor->declaration->base_name, cursor->declaration->generic_parameters, cursor_arguments);
    std::string base_base_name;
    std::vector<std::string> base_arguments;
    if (SplitGenericType(base_text, &base_base_name, &base_arguments)) {
      cursor = symbols_.Resolve(base_base_name, &klass);
      cursor_arguments = base_arguments;
    } else {
      cursor = symbols_.Resolve(base_text, &klass);
      cursor_arguments.clear();
    }
  }
  if (cursor == owner && !owner->declaration->generic_parameters.empty() &&
      cursor_arguments.size() == owner->declaration->generic_parameters.size()) {
    const std::string output_name = GenericTypeOutputName(*owner, cursor_arguments, klass, true);
    RecordNeededClass(owner);
    imports->insert(names_.Package(*owner) + "." + output_name);
    return;
  }
  imports->insert(names_.ImportPath(*owner));
}

void JavaEmitter::CollectCodeRefImports(const ClassInfo& klass, const std::string& code,
                                        std::set<std::string>* imports) {
  static const char* kKeywords[] = {"cls", "ncls", "mem"};
  std::size_t i = 0;
  while (i < code.size()) {
    if (code[i] != '#') {
      ++i;
      continue;
    }
    const std::size_t name_begin = i + 1;
    std::size_t name_end = name_begin;
    while (name_end < code.size() && IsIdentifierByte(code[name_end]))
      ++name_end;
    const std::string name = code.substr(name_begin, name_end - name_begin);
    bool matched = false;
    for (const char* keyword : kKeywords) {
      if (name != keyword || name_end >= code.size() || code[name_end] != '<')
        continue;
      std::size_t index = name_end;
      std::string content;
      if (!ReadAngleContent(code, &index, &content))
        break;
      std::string type_name = content;
      if (name == std::string("mem")) {
        const std::size_t dot = content.rfind('.');
        if (dot != std::string::npos)
          type_name = content.substr(0, dot);
      }
      if (const ClassInfo* target = symbols_.Resolve(type_name, &klass)) {
        if (target != &klass)
          imports->insert(names_.ImportPath(*target));
      }
      i = index;
      matched = true;
      break;
    }
    if (!matched)
      ++i;
  }
}

}
