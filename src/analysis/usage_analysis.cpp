#include "usage_analysis.h"

#include <algorithm>
#include <set>

#include "local_use.h"
#include "symbol_resolution.h"

namespace tiecodec {
namespace {

std::string SimpleName(const std::string& qualified_name) {
  const std::size_t dot = qualified_name.rfind('.');
  return dot == std::string::npos ? qualified_name : qualified_name.substr(dot + 1);
}

bool HasMember(const ClassDecl& declaration, const std::string& name) {
  for (const Member& member : declaration.members) {
    if (member.name == name)
      return true;
  }
  return false;
}

void CollectReference(const SymbolTable& symbols, const ClassInfo* context,
                      const std::string& type_name, std::set<const ClassInfo*>* output);

void CollectFromStatement(const SymbolTable& symbols, const ClassInfo* context, const Stmt* stmt,
                          std::set<const ClassInfo*>* output);

void CollectFromExpression(const SymbolTable& symbols, const ClassInfo* context, const Expr* expr,
                           std::set<const ClassInfo*>* output) {
  if (expr == nullptr)
    return;
  switch (expr->kind) {
    case ExprKind::kIdentifier: {
      const ClassInfo* class_ref = symbols.Resolve(expr->text, context);
      if (class_ref != nullptr)
        output->insert(class_ref);
      break;
    }
    case ExprKind::kNew:
    case ExprKind::kCast:
    case ExprKind::kIsType:
      CollectReference(symbols, context, expr->text, output);
      break;
    case ExprKind::kCall: {
      if (!expr->children.empty() && expr->children[0] != nullptr) {
        const Expr* callee = expr->children[0].get();
        const ClassInfo* owner = nullptr;
        const Member* member = nullptr;
        if (callee->kind == ExprKind::kIdentifier) {
          member = ResolveGlobalMethod(symbols, *context, callee->text, &owner);
        } else if (callee->kind == ExprKind::kFieldAccess && !callee->children.empty() &&
                   callee->children[0] != nullptr &&
                   callee->children[0]->kind == ExprKind::kIdentifier) {
          const ClassInfo* receiver = symbols.Resolve(callee->children[0]->text, context);
          if (receiver != nullptr) {
            member = FindMemberInChain(symbols, *receiver, callee->text, &owner);
          }
        }
        if (member != nullptr && owner != nullptr) {
          output->insert(owner);
          CollectReference(symbols, owner, member->type_name, output);
          for (const Parameter& parameter : member->parameters) {
            CollectReference(symbols, owner, parameter.type_name, output);
          }
        }
      }
      break;
    }
    default:
      break;
  }
  for (const ExprPtr& child : expr->children) {
    CollectFromExpression(symbols, context, child.get(), output);
  }
}

void CollectFromStatement(const SymbolTable& symbols, const ClassInfo* context, const Stmt* stmt,
                          std::set<const ClassInfo*>* output) {
  if (stmt == nullptr)
    return;
  CollectReference(symbols, context, stmt->type_name, output);
  if (stmt->kind == StmtKind::kLoopCount) {
    CollectReference(symbols, context, "整数", output);
  }
  CollectFromExpression(symbols, context, stmt->expr.get(), output);
  CollectFromExpression(symbols, context, stmt->loop_begin.get(), output);
  CollectFromExpression(symbols, context, stmt->loop_end.get(), output);
  CollectFromExpression(symbols, context, stmt->loop_step.get(), output);
  for (const SwitchCase& switch_case : stmt->cases) {
    for (const ExprPtr& value : switch_case.values) {
      CollectFromExpression(symbols, context, value.get(), output);
    }
    for (const StmtPtr& child : switch_case.body) {
      CollectFromStatement(symbols, context, child.get(), output);
    }
  }
  for (const StmtPtr& child : stmt->body)
    CollectFromStatement(symbols, context, child.get(), output);
  for (const StmtPtr& child : stmt->else_body) {
    CollectFromStatement(symbols, context, child.get(), output);
  }
}

void CollectReference(const SymbolTable& symbols, const ClassInfo* context,
                      const std::string& type_name, std::set<const ClassInfo*>* output) {
  if (type_name.empty())
    return;
  const ClassInfo* klass = symbols.Resolve(type_name, context);
  if (klass == nullptr)
    return;
  if (output->count(klass) != 0)
    return;
  output->insert(klass);
}

void CollectCodeRefTypes(const SymbolTable& symbols, const ClassInfo* context,
                         const std::string& code, std::set<const ClassInfo*>* output) {
  static const char* kKeywords[] = {"cls", "ncls", "mem"};
  std::size_t i = 0;
  while (i < code.size()) {
    if (code[i] != '#') {
      ++i;
      continue;
    }
    std::size_t name_begin = i + 1;
    std::size_t name_end = name_begin;
    while (name_end < code.size()) {
      const unsigned char c = static_cast<unsigned char>(code[name_end]);
      const bool identifier = (code[name_end] >= 'a' && code[name_end] <= 'z') ||
                              (code[name_end] >= 'A' && code[name_end] <= 'Z') ||
                              (code[name_end] >= '0' && code[name_end] <= '9') ||
                              code[name_end] == '_' || c >= 0x80;
      if (!identifier)
        break;
      ++name_end;
    }
    const std::string name = code.substr(name_begin, name_end - name_begin);
    bool matched = false;
    for (const char* keyword : kKeywords) {
      if (name != keyword || name_end >= code.size() || code[name_end] != '<')
        continue;

      std::size_t depth = 0;
      std::size_t begin = name_end + 1;
      std::size_t index = name_end;
      std::string content;
      for (; index < code.size(); ++index) {
        if (code[index] == '<') {
          ++depth;
        } else if (code[index] == '>') {
          --depth;
          if (depth == 0) {
            content = code.substr(begin, index - begin);
            ++index;
            break;
          }
        }
      }
      if (depth != 0)
        break;
      std::string type_name = content;
      if (name == std::string("mem")) {
        const std::size_t dot = content.rfind('.');
        if (dot != std::string::npos)
          type_name = content.substr(0, dot);
      }
      CollectReference(symbols, context, type_name, output);
      i = index;
      matched = true;
      break;
    }
    if (!matched)
      ++i;
  }
}

void CollectFromMember(const SymbolTable& symbols, const ClassInfo* owner, const Member& member,
                       std::set<const ClassInfo*>* output) {
  CollectReference(symbols, owner, member.type_name, output);
  for (const Parameter& parameter : member.parameters) {
    CollectReference(symbols, owner, parameter.type_name, output);
    CollectFromExpression(symbols, owner, parameter.default_value.get(), output);
  }
  CollectFromExpression(symbols, owner, member.expression_body.get(), output);
  if (!member.code_text.empty()) {
    CollectCodeRefTypes(symbols, owner, member.code_text, output);
  }
  const std::set<std::string> used_locals = UsedLocalNames(member);
  for (const StmtPtr& statement : member.body) {
    if (statement != nullptr && statement->kind == StmtKind::kCode) {
      CollectCodeRefTypes(symbols, owner, statement->code_text, output);
    }
    if (!IsLocalDeclarationUsed(*statement, used_locals))
      continue;
    CollectFromStatement(symbols, owner, statement.get(), output);
  }
}

struct UsedMemberCollector {
  const SymbolTable& symbols;
  std::set<const Member*>* used = nullptr;

  std::vector<std::pair<const ClassInfo*, const Member*>>* virtual_overrides = nullptr;
  std::vector<std::pair<const ClassInfo*, const Member*>> queue;

  void MarkUsed(const ClassInfo* owner, const Member* member) {
    if (owner == nullptr || member == nullptr)
      return;
    if (!used->insert(member).second)
      return;
    queue.emplace_back(owner, member);
  }

  std::string FieldTypeOf(const ClassInfo& klass, const std::string& name) const {
    const ClassInfo* current = &klass;
    std::set<const ClassInfo*> visited;
    while (current != nullptr && current->declaration != nullptr &&
           visited.insert(current).second) {
      for (const Member& member : current->declaration->members) {
        if (member.name == name)
          return member.type_name;
      }
      if (current->declaration->base_name.empty())
        break;
      current = symbols.Resolve(current->declaration->base_name, current);
    }
    return {};
  }

  const ClassInfo* ReceiverClassOf(const ClassInfo& context,
                                   std::map<std::string, std::string>* locals,
                                   const Expr* receiver) const {
    if (receiver == nullptr)
      return nullptr;
    if (receiver->kind != ExprKind::kIdentifier) {
      const std::string type = StaticTypeOf(context, locals, receiver);
      if (type.empty())
        return nullptr;
      return symbols.Resolve(type, &context);
    }
    const auto local = locals->find(receiver->text);
    if (local != locals->end())
      return symbols.Resolve(local->second, &context);
    const std::string field_type = FieldTypeOf(context, receiver->text);
    if (!field_type.empty())
      return symbols.Resolve(field_type, &context);
    return nullptr;
  }

  const Member* LookupMember(const ClassInfo& klass, const std::string& name,
                             const ClassInfo** owner) const {
    return FindMemberInChain(symbols, klass, name, owner);
  }

  const Member* LookupMemberOfKind(const ClassInfo& klass, const std::string& name,
                                   MemberKind preferred, const ClassInfo** owner) const {
    const ClassInfo* current = &klass;
    std::set<const ClassInfo*> visited;
    while (current != nullptr && current->declaration != nullptr &&
           visited.insert(current).second) {
      const Member* fallback = nullptr;
      for (const Member& member : current->declaration->members) {
        if (member.name != name)
          continue;
        if (member.kind == preferred) {
          if (owner != nullptr)
            *owner = current;
          return &member;
        }
        if (fallback == nullptr)
          fallback = &member;
      }
      if (fallback != nullptr) {
        if (owner != nullptr)
          *owner = current;
        return fallback;
      }
      if (current->declaration->base_name.empty())
        break;
      current = symbols.Resolve(current->declaration->base_name, current);
    }
    return nullptr;
  }

  const Member* LookupBaseEvent(const ClassInfo& klass, const std::string& name,
                                const ClassInfo** owner) const {
    if (klass.declaration == nullptr || klass.declaration->base_name.empty())
      return nullptr;
    const ClassInfo* base = symbols.Resolve(klass.declaration->base_name, &klass);
    if (base == nullptr)
      return nullptr;
    return FindMemberInChain(symbols, *base, name, owner);
  }

  std::string StaticTypeOf(const ClassInfo& context, std::map<std::string, std::string>* locals,
                           const Expr* expression) const {
    if (expression == nullptr)
      return {};
    switch (expression->kind) {
      case ExprKind::kIdentifier: {
        const auto local = locals->find(expression->text);
        if (local != locals->end())
          return local->second;
        const std::string field_type = FieldTypeOf(context, expression->text);
        if (!field_type.empty())
          return field_type;
        return {};
      }
      case ExprKind::kLiteral: {
        if (expression->text == "真" || expression->text == "假")
          return "逻辑型";
        if (!expression->raw.empty() && (expression->raw[0] == '"' || expression->raw[0] == '[')) {
          return "文本";
        }
        if (!expression->raw.empty() && expression->raw[0] == '\'')
          return "字符";
        if (expression->text.find('.') != std::string::npos)
          return "小数";
        return "整数";
      }
      case ExprKind::kFieldAccess: {
        const ClassInfo* receiver =
            ReceiverClassOf(context, locals,
                            expression->children.empty() ? nullptr : expression->children[0].get());
        if (receiver != nullptr) {
          const ClassInfo* owner = nullptr;
          if (const Member* member =
                  FindMemberInChain(symbols, *receiver, expression->text, &owner)) {
            return member->type_name;
          }
        }
        return {};
      }
      default:
        return {};
    }
    return {};
  }

  void ScanExpression(const ClassInfo& context, std::map<std::string, std::string>* locals,
                      const Expr* expression) {
    if (expression == nullptr)
      return;
    switch (expression->kind) {
      case ExprKind::kIdentifier: {
        if (locals->count(expression->text) != 0)
          break;
        const ClassInfo* owner = nullptr;
        if (const Member* member = LookupMember(context, expression->text, &owner)) {
          MarkUsed(owner, member);
        } else if (const Member* global =
                       FindGlobalClassMember(symbols, context, expression->text, &owner)) {
          MarkUsed(owner, global);
        } else if (const Member* global =
                       ResolveGlobalMethod(symbols, context, expression->text, &owner)) {
          MarkUsed(owner, global);
        }
        break;
      }
      case ExprKind::kBinary: {
        if (expression->children.size() == 2 && expression->children[0] != nullptr) {
          const std::string left_type =
              StaticTypeOf(context, locals, expression->children[0].get());
          const ClassInfo* left_class = symbols.Resolve(left_type, &context);
          if (left_class != nullptr) {
            const ClassInfo* owner = nullptr;
            if (const Member* overload =
                    FindMemberInChain(symbols, *left_class, expression->text, &owner)) {
              MarkUsed(owner, overload);
            }
          }
        }
        break;
      }
      case ExprKind::kAssign: {
        if (expression->children.size() == 2 && expression->children[0] != nullptr &&
            expression->children[1] != nullptr) {
          const Expr* target = expression->children[0].get();
          if (target->kind == ExprKind::kIdentifier && locals->count(target->text) == 0) {
            const ClassInfo* owner = nullptr;
            if (const Member* property =
                    LookupMemberOfKind(context, target->text, MemberKind::kPropertyWrite, &owner)) {
              MarkUsed(owner, property);
            }
          }
          if (target->kind == ExprKind::kFieldAccess && !target->children.empty() &&
              target->children[0] != nullptr &&
              target->children[0]->kind == ExprKind::kIdentifier) {
            const ClassInfo* receiver_class =
                ReceiverClassOf(context, locals, target->children[0].get());
            if (receiver_class == nullptr) {
              receiver_class = symbols.Resolve(target->children[0]->text, &context);
            }
            if (receiver_class != nullptr) {
              const ClassInfo* owner = nullptr;
              if (const Member* member = LookupMemberOfKind(*receiver_class, target->text,
                                                            MemberKind::kPropertyWrite, &owner)) {
                MarkUsed(owner, member);
              }
            }
          }
          ScanExpression(context, locals, expression->children[1].get());
          break;
        }
        break;
      }
      case ExprKind::kFieldAccess: {
        if (expression->children.empty() || expression->children[0] == nullptr)
          break;
        const Expr* receiver = expression->children[0].get();
        const ClassInfo* receiver_class = ReceiverClassOf(context, locals, receiver);
        if (receiver_class == nullptr && receiver->kind == ExprKind::kIdentifier) {
          receiver_class = symbols.Resolve(receiver->text, &context);
        }
        if (receiver_class != nullptr) {
          const ClassInfo* owner = nullptr;
          if (const Member* member = LookupMemberOfKind(*receiver_class, expression->text,
                                                        MemberKind::kPropertyRead, &owner)) {
            MarkUsed(owner, member);
          } else if (const Member* field =
                         FindMemberInChain(symbols, *receiver_class, expression->text, &owner)) {
            MarkUsed(owner, field);
          }
        }
        break;
      }
      case ExprKind::kCall: {
        if (!expression->children.empty() && expression->children[0] != nullptr) {
          const Expr* callee = expression->children[0].get();
          const ClassInfo* owner = nullptr;
          const Member* member = nullptr;
          if (callee->kind == ExprKind::kIdentifier) {
            member = ResolveGlobalMethod(symbols, context, callee->text, &owner);
          } else if (callee->kind == ExprKind::kFieldAccess && !callee->children.empty() &&
                     callee->children[0] != nullptr &&
                     callee->children[0]->kind == ExprKind::kIdentifier) {
            const ClassInfo* receiver = ReceiverClassOf(context, locals, callee->children[0].get());
            if (receiver == nullptr) {
              receiver = symbols.Resolve(callee->children[0]->text, &context);
            }
            if (receiver != nullptr) {
              member = LookupMemberOfKind(*receiver, callee->text, MemberKind::kMethod, &owner);
            }
          }
          if (member != nullptr)
            MarkUsed(owner, member);
        }
        break;
      }
      default:
        break;
    }
    for (const ExprPtr& child : expression->children) {
      ScanExpression(context, locals, child.get());
    }
  }

  void ScanStatement(const ClassInfo& context, std::map<std::string, std::string>* locals,
                     const Stmt* statement) {
    if (statement == nullptr)
      return;
    if (statement->kind == StmtKind::kVariable && !statement->name.empty() &&
        !statement->type_name.empty()) {
      (*locals)[statement->name] = statement->type_name;
    }
    if (statement->kind == StmtKind::kVariable && statement->expr != nullptr &&
        !statement->type_name.empty()) {
      const ClassInfo* declared = symbols.Resolve(statement->type_name, &context);
      if (declared != nullptr && statement->expr->kind != ExprKind::kNew) {
        const std::string value_type = StaticTypeOf(context, locals, statement->expr.get());
        const ClassInfo* value_class = symbols.Resolve(value_type, &context);
        if (value_class != declared) {
          const ClassInfo* owner = nullptr;
          if (const Member* assignment = FindMemberInChain(symbols, *declared, "=", &owner)) {
            if (assignment->kind == MemberKind::kMethod)
              MarkUsed(owner, assignment);
          }
        }
      }
    }
    if (statement->kind == StmtKind::kEventBlock && !statement->event_name.empty()) {
      const ClassInfo* owner = nullptr;
      if (const Member* base_event = LookupBaseEvent(context, statement->event_name, &owner)) {
        MarkUsed(owner, base_event);
      }
    }
    ScanExpression(context, locals, statement->expr.get());
    ScanExpression(context, locals, statement->loop_begin.get());
    ScanExpression(context, locals, statement->loop_end.get());
    ScanExpression(context, locals, statement->loop_step.get());
    for (const SwitchCase& switch_case : statement->cases) {
      for (const ExprPtr& value : switch_case.values) {
        ScanExpression(context, locals, value.get());
      }
      for (const StmtPtr& child : switch_case.body) {
        ScanStatement(context, locals, child.get());
      }
    }
    for (const StmtPtr& child : statement->body) {
      ScanStatement(context, locals, child.get());
    }
    for (const StmtPtr& child : statement->else_body) {
      ScanStatement(context, locals, child.get());
    }
  }

  void ScanCodeText(const ClassInfo& context, const std::string& code) {
    std::size_t i = 0;
    while (i < code.size()) {
      if (code[i] != '#') {
        ++i;
        continue;
      }
      std::size_t name_begin = i + 1;
      std::size_t name_end = name_begin;
      while (name_end < code.size()) {
        const unsigned char c = static_cast<unsigned char>(code[name_end]);
        const bool identifier = (code[name_end] >= 'a' && code[name_end] <= 'z') ||
                                (code[name_end] >= 'A' && code[name_end] <= 'Z') ||
                                (code[name_end] >= '0' && code[name_end] <= '9') ||
                                code[name_end] == '_' || c >= 0x80;
        if (!identifier)
          break;
        ++name_end;
      }
      const std::string keyword = code.substr(name_begin, name_end - name_begin);
      const bool has_angle = name_end < code.size() && code[name_end] == '<';
      if ((keyword == "get" || keyword == "set" || keyword == "mem") && has_angle) {
        std::size_t depth = 0;
        std::size_t begin = name_end + 1;
        std::size_t index = name_end;
        std::string content;
        for (; index < code.size(); ++index) {
          if (code[index] == '<')
            ++depth;
          else if (code[index] == '>') {
            --depth;
            if (depth == 0) {
              content = code.substr(begin, index - begin);
              ++index;
              break;
            }
          }
        }
        if (keyword == "mem") {
          const std::size_t dot = content.rfind('.');
          if (dot != std::string::npos) {
            const ClassInfo* owner_class = symbols.Resolve(content.substr(0, dot), &context);
            if (owner_class != nullptr) {
              const ClassInfo* owner = nullptr;
              if (const Member* member =
                      FindMemberInChain(symbols, *owner_class, content.substr(dot + 1), &owner)) {
                MarkUsed(owner, member);
              }
            }
            i = index;
            continue;
          }
        }
        const ClassInfo* owner = nullptr;
        const MemberKind preferred =
            keyword == "set" ? MemberKind::kPropertyWrite
                             : (keyword == "get" ? MemberKind::kPropertyRead : MemberKind::kMethod);
        if (const Member* member = LookupMemberOfKind(context, content, preferred, &owner)) {
          MarkUsed(owner, member);
        }
        i = index;
        continue;
      }
      if (!keyword.empty() && keyword != "this" && keyword != "cls" && keyword != "ncls" &&
          keyword != "expr" && keyword != "sys") {
        const ClassInfo* owner = nullptr;
        if (const Member* member = FindMemberInChain(symbols, context, keyword, &owner)) {
          MarkUsed(owner, member);

          if (virtual_overrides != nullptr && owner != &context) {
            const bool is_virtual =
                std::any_of(member->annotations.begin(), member->annotations.end(),
                            [](const Annotation& annotation) {
                              return annotation.name == "虚拟事件" || annotation.name == "虚拟方法";
                            });
            if (is_virtual && member->kind == MemberKind::kMethod) {
              virtual_overrides->emplace_back(&context, member);
            }
          }
        }
      }
      i = name_end > i ? name_end : i + 1;
    }
  }

  void ScanMember(const ClassInfo& owner, const Member& member) {
    if (member.kind == MemberKind::kEventBlock && !member.name.empty()) {
      const ClassInfo* event_owner = nullptr;
      if (const Member* base_event = LookupBaseEvent(owner, member.name, &event_owner)) {
        MarkUsed(event_owner, base_event);
      }

      if (!member.event_object.empty()) {
        if (const Member* receiver = LookupMember(owner, member.event_object, nullptr)) {
          MarkUsed(&owner, receiver);
        }
        std::string receiver_type;
        const ClassInfo* cursor = &owner;
        std::set<const ClassInfo*> visited;
        while (cursor != nullptr && cursor->declaration != nullptr &&
               visited.insert(cursor).second) {
          for (const Member& candidate : cursor->declaration->members) {
            if (candidate.name == member.event_object)
              receiver_type = candidate.type_name;
          }
          if (!receiver_type.empty() || cursor->declaration->base_name.empty())
            break;
          cursor = symbols.Resolve(cursor->declaration->base_name, cursor);
        }
        const ClassInfo* receiver_class = symbols.Resolve(receiver_type, &owner);
        if (receiver_class != nullptr) {
          const ClassInfo* definition_owner = nullptr;
          if (const Member* definition =
                  FindMemberInChain(symbols, *receiver_class, member.name, &definition_owner)) {
            if (definition->kind == MemberKind::kEventDefinition) {
              MarkUsed(definition_owner, definition);

              for (const Annotation& annotation : definition->annotations) {
                if (annotation.name != "属性需求")
                  continue;
                const std::string property = !annotation.arguments.empty()
                                                 ? annotation.arguments.front().value
                                                 : annotation.StringArgument("参数");
                if (property.empty())
                  continue;
                const ClassInfo* cursor2 = definition_owner;
                std::set<const ClassInfo*> seen;
                while (cursor2 != nullptr && cursor2->declaration != nullptr &&
                       seen.insert(cursor2).second) {
                  const Member* writer = nullptr;
                  for (const Member& candidate : cursor2->declaration->members) {
                    if (candidate.kind == MemberKind::kPropertyWrite &&
                        candidate.name == property) {
                      writer = &candidate;
                      break;
                    }
                  }
                  if (writer != nullptr) {
                    MarkUsed(cursor2, writer);

                    for (const Member& candidate : cursor2->declaration->members) {
                      if (candidate.kind == MemberKind::kPropertyRead &&
                          candidate.name == property) {
                        MarkUsed(cursor2, &candidate);
                        break;
                      }
                    }
                    break;
                  }
                  if (cursor2->declaration->base_name.empty())
                    break;
                  cursor2 = symbols.Resolve(cursor2->declaration->base_name, cursor2);
                }
              }
            }
          }
        }
      }
    }
    if (!member.code_text.empty())
      ScanCodeText(owner, member.code_text);
    for (const StmtPtr& statement : member.body) {
      if (statement != nullptr && statement->kind == StmtKind::kCode) {
        ScanCodeText(owner, statement->code_text);
      }
    }
    std::map<std::string, std::string> locals;
    for (const Parameter& parameter : member.parameters) {
      if (!parameter.type_name.empty())
        locals[parameter.name] = parameter.type_name;
    }
    if (member.kind == MemberKind::kEventBlock && !member.event_object.empty()) {
    }
    ScanExpression(owner, &locals, member.expression_body.get());
    const std::set<std::string> used_locals = UsedLocalNames(member);
    for (const StmtPtr& statement : member.body) {
      if (!IsLocalDeclarationUsed(*statement, used_locals))
        continue;
      ScanStatement(owner, &locals, statement.get());
    }
  }

  void Run(const ClassInfo* entry, const std::vector<const ClassInfo*>& emitted_classes,
           const std::vector<RequiredMember>& required_members) {
    for (const ClassInfo* klass : emitted_classes) {
      if (klass == nullptr || klass->declaration == nullptr)
        continue;
      for (const Member& member : klass->declaration->members) {
        if (member.kind == MemberKind::kCodeBlock)
          MarkUsed(klass, &member);
      }
    }

    const auto seed_forced = [this](const ClassInfo* klass) {
      if (klass == nullptr || klass->declaration == nullptr)
        return;
      for (const Member& member : klass->declaration->members) {
        if (member.kind == MemberKind::kEventBlock) {
          MarkUsed(klass, &member);
        }
        const bool forced = std::any_of(
            member.annotations.begin(), member.annotations.end(), [](const Annotation& annotation) {
              return annotation.name == "强制输出" || annotation.name == "主题配置";
            });
        if (forced)
          MarkUsed(klass, &member);
      }
    };
    for (const ClassInfo* klass : emitted_classes)
      seed_forced(klass);

    for (const ClassInfo* klass : emitted_classes) {
      if (klass == nullptr || klass->declaration == nullptr)
        continue;
      for (const Member& member : klass->declaration->members) {
        const bool component_config = std::any_of(
            member.annotations.begin(), member.annotations.end(), [](const Annotation& annotation) {
              return annotation.name == "组件配置" || annotation.name == "布局配置";
            });
        if (component_config)
          MarkUsed(klass, &member);
      }
    }

    for (const ClassInfo* klass : emitted_classes) {
      if (klass == nullptr || klass->declaration == nullptr)
        continue;
      for (const Member& member : klass->declaration->members) {
        const bool virtual_method =
            std::any_of(member.annotations.begin(), member.annotations.end(),
                        [](const Annotation& annotation) { return annotation.name == "虚拟方法"; });
        if (virtual_method)
          MarkUsed(klass, &member);
      }
    }

    for (const RequiredMember& required : required_members) {
      for (const std::string& qualified_name : symbols.class_names()) {
        if (SimpleName(qualified_name) != required.class_name)
          continue;
        const ClassInfo* klass = symbols.Find(qualified_name);
        if (klass == nullptr || klass->declaration == nullptr)
          continue;
        const ClassInfo* owner = nullptr;
        if (const Member* member =
                FindMemberInChain(symbols, *klass, required.member_name, &owner)) {
          MarkUsed(owner, member);
        }
        break;
      }
    }
    (void)entry;

    while (!queue.empty()) {
      const auto item = queue.back();
      queue.pop_back();
      ScanMember(*item.first, *item.second);
    }
  }
};

}  // namespace

std::set<const Member*> ComputeUsedMembers(
    const SymbolTable& symbols, const EnvironmentContract& contract, const ClassInfo* entry,
    const std::vector<const ClassInfo*>& emitted_classes,
    std::vector<std::pair<const ClassInfo*, const Member*>>* virtual_overrides) {
  std::set<const Member*> used;
  UsedMemberCollector collector{symbols, &used, virtual_overrides, {}};
  collector.Run(entry, emitted_classes, contract.RequiredMembers());
  return used;
}

OutputPlan ComputeOutputPlan(const SymbolTable& symbols, const EnvironmentContract& contract,
                             const ClassInfo* entry) {
  OutputPlan plan;
  if (entry == nullptr)
    return plan;
  const std::vector<RequiredMember>& required_members = contract.RequiredMembers();

  std::set<const ClassInfo*> classes;
  const ClassInfo* current = entry;
  while (current != nullptr && current->declaration != nullptr) {
    classes.insert(current);
    if (current->declaration->base_name.empty())
      break;
    current = symbols.Resolve(current->declaration->base_name, current);
  }

  for (const auto& entry_classes : symbols.classes()) {
    const ClassInfo& klass = entry_classes.second;
    if (klass.declaration == nullptr)
      continue;
    bool forced = false;
    for (const Annotation& annotation : klass.declaration->annotations) {
      if (annotation.name == "全局应用")
        forced = true;
      if (annotation.name != "强制输出")
        continue;
      std::string value = annotation.StringArgument("参数");
      if (value.empty() && !annotation.arguments.empty()) {
        value = annotation.arguments.front().value;
      }
      if (value == "真" || value == "true" || value == "1")
        forced = true;
    }
    if (!forced)
      continue;
    const ClassInfo* cursor = &klass;
    while (cursor != nullptr && cursor->declaration != nullptr) {
      classes.insert(cursor);
      if (cursor->declaration->base_name.empty())
        break;
      cursor = symbols.Resolve(cursor->declaration->base_name, cursor);
    }
  }

  const auto add_with_bases = [&](const ClassInfo* klass, std::set<const ClassInfo*>* target) {
    const ClassInfo* cursor = klass;
    while (cursor != nullptr && cursor->declaration != nullptr && target->insert(cursor).second) {
      if (cursor->declaration->base_name.empty())
        break;
      cursor = symbols.Resolve(cursor->declaration->base_name, cursor);
    }
  };

  while (true) {
    plan.classes.assign(classes.begin(), classes.end());
    std::sort(plan.classes.begin(), plan.classes.end(), [](const ClassInfo* a, const ClassInfo* b) {
      return a->qualified_name < b->qualified_name;
    });
    plan.virtual_overrides.clear();
    plan.members =
        ComputeUsedMembers(symbols, contract, entry, plan.classes, &plan.virtual_overrides);
    {
      bool added = false;
      for (const Member* member : plan.members) {
        if (member == nullptr)
          continue;
        const bool theme =
            std::any_of(member->annotations.begin(), member->annotations.end(),
                        [](const Annotation& annotation) { return annotation.name == "主题配置"; });
        if (!theme)
          continue;
        const ClassInfo* owner = nullptr;
        for (const ClassInfo* klass : plan.classes) {
          if (klass == nullptr || klass->declaration == nullptr)
            continue;
          for (const Member& candidate : klass->declaration->members) {
            if (&candidate == member)
              owner = klass;
          }
          if (owner != nullptr)
            break;
        }
        if (owner == nullptr)
          continue;
        const ClassInfo* type = symbols.Resolve(member->type_name, owner);
        if (type == nullptr)
          continue;
        const std::size_t before = classes.size();
        add_with_bases(type, &classes);
        if (classes.size() != before)
          added = true;
      }
      if (added)
        continue;
    }

    for (const ClassInfo* klass : plan.classes) {
      if (klass == nullptr || klass->declaration == nullptr)
        continue;
      for (const Member& member : klass->declaration->members) {
        const Annotation* config = nullptr;
        for (const Annotation& annotation : member.annotations) {
          if (annotation.name == "组件配置" || annotation.name == "布局配置")
            config = &annotation;
        }
        if (config == nullptr)
          continue;
        const ClassInfo* type_class = symbols.Resolve(member.type_name, klass);
        if (type_class == nullptr)
          continue;
        std::string parent_value;
        for (const AnnotationArgument& argument : config->arguments) {
          if (argument.name == "父布局")
            parent_value = argument.value;
          if (argument.name == "根布局")
            continue;
        }
        for (const AnnotationArgument& argument : config->arguments) {
          std::string table = argument.value;
          if (!argument.name.empty() && argument.name != "根布局" && argument.name != "父布局") {
            table.clear();
          }
          std::size_t begin = 0;
          while (begin < table.size()) {
            std::size_t end = table.find(',', begin);
            if (end == std::string::npos)
              end = table.size();
            const std::string entry = table.substr(begin, end - begin);
            begin = end + 1;
            const std::size_t equals = entry.find('=');
            if (equals == std::string::npos)
              continue;
            std::string key = entry.substr(0, equals);
            while (!key.empty() && (key.front() == ' ' || key.front() == '{'))
              key.erase(key.begin());
            while (!key.empty() && (key.back() == ' ' || key.back() == '}'))
              key.pop_back();
            if (key.empty() || key == "根布局" || key == "父布局")
              continue;
            if (key[0] == '$') {
              std::string property_name = key.substr(1);
              while (!property_name.empty() &&
                     (property_name.front() == ' ' || property_name.front() == '\t')) {
                property_name.erase(property_name.begin());
              }
              if (property_name.empty() || parent_value.empty())
                continue;
              const Member* parent_member =
                  FindMemberInChain(symbols, *klass, parent_value, nullptr);
              if (parent_member == nullptr)
                continue;
              const ClassInfo* parent_type = symbols.Resolve(parent_member->type_name, klass);
              if (parent_type == nullptr)
                continue;
              const ClassInfo* owner = nullptr;
              if (const Member* property =
                      FindMemberInChain(symbols, *parent_type, property_name, &owner)) {
                plan.members.insert(property);
              }
              continue;
            }
            const ClassInfo* owner = nullptr;
            if (const Member* property = FindMemberInChain(symbols, *type_class, key, &owner)) {
              plan.members.insert(property);
            }
          }
        }
      }
    }

    std::set<const ClassInfo*> references;
    for (const ClassInfo* klass : plan.classes) {
      if (klass == nullptr || klass->declaration == nullptr)
        continue;
      for (const Member& member : klass->declaration->members) {
        if (plan.members.count(&member) == 0)
          continue;
        CollectFromMember(symbols, klass, member, &references);
      }
    }

    for (const ClassInfo* klass : plan.classes) {
      if (klass == nullptr || klass->declaration == nullptr)
        continue;
      const auto collect_from_annotations = [&](const std::vector<Annotation>& annotations) {
        for (const Annotation& annotation : annotations) {
          for (const AnnotationArgument& argument : annotation.arguments) {
            const std::string& text = argument.value;
            const std::size_t dot = text.rfind('.');
            if (dot == std::string::npos)
              continue;
            const ClassInfo* owner = symbols.Resolve(text.substr(0, dot), klass);
            if (owner == nullptr)
              continue;
            references.insert(owner);
            const ClassInfo* member_owner = nullptr;
            if (const Member* member =
                    FindMemberInChain(symbols, *owner, text.substr(dot + 1), &member_owner)) {
              plan.members.insert(member);
            }
          }
        }
      };
      collect_from_annotations(klass->declaration->annotations);

      for (const Member& member : klass->declaration->members) {
        if (plan.members.count(&member) == 0)
          continue;
        collect_from_annotations(member.annotations);
      }
    }

    bool changed = false;
    for (const ClassInfo* reference : references) {
      const std::size_t before = classes.size();
      add_with_bases(reference, &classes);
      if (classes.size() != before)
        changed = true;
    }
    if (!changed)
      break;
  }

  const auto declared = [](const ClassInfo& klass, const std::string& name) {
    if (klass.declaration == nullptr)
      return false;
    for (const Member& member : klass.declaration->members) {
      if (member.name == name)
        return true;
    }
    return false;
  };
  for (const RequiredMember& required : required_members) {
    const ClassInfo* declaring = nullptr;
    const Member* declaring_member = nullptr;
    for (const std::string& qualified_name : symbols.class_names()) {
      if (SimpleName(qualified_name) != required.class_name)
        continue;
      const ClassInfo* klass = symbols.Find(qualified_name);
      if (klass == nullptr)
        continue;
      const ClassInfo* owner = nullptr;
      if (const Member* member = FindMemberInChain(symbols, *klass, required.member_name, &owner)) {
        declaring = owner;
        declaring_member = member;
      }
      break;
    }
    if (declaring == nullptr || declaring_member == nullptr)
      continue;
    const bool is_virtual_event =
        std::any_of(declaring_member->annotations.begin(), declaring_member->annotations.end(),
                    [](const Annotation& annotation) { return annotation.name == "虚拟事件"; });
    if (!is_virtual_event)
      continue;
    for (const ClassInfo* klass : plan.classes) {
      if (klass == nullptr || klass->declaration == nullptr || klass == declaring)
        continue;

      bool descends = false;
      const ClassInfo* cursor = klass;
      std::set<const ClassInfo*> visited;
      while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
        if (cursor == declaring) {
          descends = true;
          break;
        }
        if (cursor->declaration->base_name.empty())
          break;
        cursor = symbols.Resolve(cursor->declaration->base_name, cursor);
      }
      if (!descends || declared(*klass, declaring_member->name))
        continue;
      const auto duplicate =
          std::find_if(plan.virtual_overrides.begin(), plan.virtual_overrides.end(),
                       [klass, declaring_member](const auto& entry) {
                         return entry.first == klass && entry.second == declaring_member;
                       });
      if (duplicate == plan.virtual_overrides.end()) {
        plan.virtual_overrides.emplace_back(klass, declaring_member);
      }
    }
  }
  return plan;
}
}  // namespace tiecodec
