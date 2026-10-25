#include "java_emitter.h"

#include "java_const.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>

#include "internal/helpers.h"
#include "java_effect_types.h"
#include "symbol_resolution.h"

namespace tiecodec {

using namespace detail;
std::string JavaEmitter::MemberTypeOf(const ClassInfo& klass, const std::string& name) const {
  const ClassInfo* current = &klass;
  const std::size_t max_depth = symbols_.class_count() + 1;
  for (std::size_t depth = 0; current != nullptr && current->declaration != nullptr; ++depth) {
    if (depth == max_depth)
      return {};
    const auto member = current->member_by_name.find(name);
    if (member != current->member_by_name.end())
      return member->second->type_name;
    if (current->declaration->base_name.empty())
      break;
    current = symbols_.Resolve(current->declaration->base_name, current);
  }
  return {};
}

const Member* JavaEmitter::ResolveCalledMember(const ClassInfo& klass, const Expr& callee,
                                               const ClassInfo** owner) const {
  if (callee.kind == ExprKind::kIdentifier) {
    return ResolveGlobalMethod(symbols_, klass, callee.text, owner);
  }
  if (callee.kind == ExprKind::kFieldAccess && !callee.children.empty() &&
      callee.children[0] != nullptr) {
    const ClassInfo* receiver_class = ReceiverClass(klass, *callee.children[0]);
    if (receiver_class != nullptr) {
      return FindMemberInChain(symbols_, *receiver_class, callee.text, owner);
    }
  }
  return nullptr;
}

std::string JavaEmitter::CallArguments(const ClassInfo& call_site,
                                       const ClassInfo& declaration_site, const Member& member,
                                       const Expr& call, bool skip_first_argument) {
  std::string arguments;
  std::size_t argument_index = 1;
  for (std::size_t i = 0; i < member.parameters.size(); ++i) {
    if (!arguments.empty())
      arguments += ", ";
    const Expr* provided =
        argument_index < call.children.size() ? call.children[argument_index].get() : nullptr;
    if (provided != nullptr) {
      if (provided->kind == ExprKind::kMulti) {
        for (std::size_t j = 0; j < provided->children.size(); ++j) {
          if (j > 0)
            arguments += ", ";
          if (provided->children[j] != nullptr) {
            arguments += EmitExpression(call_site, *provided->children[j]);
          }
        }
      } else {
        arguments += EmitConverted(call_site, member.parameters[i].type_name, *provided);
      }
      ++argument_index;
    } else if (member.parameters[i].default_value != nullptr) {
      arguments += EmitExpression(declaration_site, *member.parameters[i].default_value);
    } else {
      arguments += "null";
    }
  }
  (void)skip_first_argument;
  return arguments;
}

const ClassInfo* JavaEmitter::ReceiverClass(const ClassInfo& klass, const Expr& receiver) const {
  if (receiver.kind == ExprKind::kIdentifier) {
    const auto local = local_types_.find(receiver.text);
    if (local != local_types_.end()) {
      return symbols_.Resolve(local->second, &klass);
    }
    const std::string member_type = MemberTypeOf(klass, receiver.text);
    if (!member_type.empty())
      return symbols_.Resolve(member_type, &klass);
    return symbols_.Resolve(receiver.text, &klass);
  }
  return nullptr;
}

bool JavaEmitter::TryEmitPropertyWrite(const ClassInfo& klass, const Expr& target,
                                       const Expr& value, std::string* out_text) {
  if (target.kind == ExprKind::kIdentifier && local_types_.count(target.text) == 0 &&
      current_parameters_.count(target.text) == 0) {
    const ClassInfo* property_owner = nullptr;
    const Member* property = FindMemberInChain(symbols_, klass, target.text, &property_owner);
    if (property != nullptr && property->kind == MemberKind::kPropertyWrite) {
      const std::string receiver =
          alias_static_context_ ? "__this_obj" : names_.ClassName(klass) + ".this";
      if (property_owner->HasExternalType("java")) {
        const std::string converted =
            property->parameters.empty()
                ? EmitExpression(klass, value)
                : EmitConverted(klass, property->parameters.front().type_name, value);
        *out_text = OwnerCallName(klass, *property_owner, nullptr) + "." +
                    names_.MemberName(*property) + "(" + receiver + ", " + converted + ")";
      } else {
        const std::string converted =
            property->parameters.empty()
                ? EmitExpression(klass, value)
                : EmitConverted(klass, property->parameters.front().type_name, value);
        *out_text = names_.MemberName(*property) + "(" + converted + ")";
      }
      return true;
    }
  }
  if (target.kind != ExprKind::kFieldAccess || target.children.empty() ||
      target.children[0] == nullptr) {
    return false;
  }
  const ClassInfo* receiver_class = ReceiverClass(klass, *target.children[0]);
  if (receiver_class == nullptr || receiver_class->declaration == nullptr)
    return false;
  const ClassInfo* cursor = receiver_class;
  std::set<const ClassInfo*> visited;
  while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
    for (const Member& member : cursor->declaration->members) {
      if (member.kind != MemberKind::kPropertyWrite || member.name != target.text)
        continue;
      const std::string receiver = EmitExpression(klass, *target.children[0]);
      if (cursor->HasExternalType("java")) {
        const std::string converted =
            member.parameters.empty()
                ? EmitExpression(klass, value)
                : EmitConverted(klass, member.parameters.front().type_name, value);
        *out_text = OwnerCallName(klass, *cursor, target.children[0].get()) + "." +
                    names_.MemberName(member) + "(" + receiver + ", " + converted + ")";
      } else {
        const std::string converted =
            member.parameters.empty()
                ? EmitExpression(klass, value)
                : EmitConverted(klass, member.parameters.front().type_name, value);
        *out_text = receiver + "." + names_.MemberName(member) + "(" + converted + ")";
      }
      return true;
    }
    if (cursor->declaration->base_name.empty())
      break;
    cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
  }
  return false;
}

std::string JavaEmitter::EmitExpression(const ClassInfo& klass, const Expr& expression) {
  if (expression.kind == ExprKind::kBinary || expression.kind == ExprKind::kParen) {
    ConstantValue constant;
    if (FoldConstant(expression, &constant))
      return FormatConstant(constant);
  }
  switch (expression.kind) {
    case ExprKind::kLiteral: {
      if (expression.text == "真")
        return "true";
      if (expression.text == "假")
        return "false";
      const bool numeric_literal =
          expression.raw == expression.text && !expression.text.empty() &&
          (expression.text[0] == '-' || (expression.text[0] >= '0' && expression.text[0] <= '9'));
      if (numeric_literal && expression.text.find('.') != std::string::npos) {
        std::string text = expression.text;
        const char last = text.back();
        if (last == 'f' || last == 'F') {
          text.back() = 'F';
          return text;
        }
        if (last == 'd' || last == 'D') {
          text.back() = 'D';
          return text;
        }
        return text + "D";
      }
      if (!expression.raw.empty() && (expression.raw[0] == '"' || expression.raw[0] == '\'')) {
        const char quote = expression.raw[0];
        std::string inner =
            expression.raw.substr(1, expression.raw.size() >= 2 ? expression.raw.size() - 2 : 0);
        for (int pass = 0; pass < plus_left_escape_passes_; ++pass) {
          std::string doubled;
          doubled.reserve(inner.size());
          for (char c : inner) {
            if (c == '\\')
              doubled += "\\\\";
            else
              doubled.push_back(c);
          }
          inner = std::move(doubled);
        }
        std::string escaped(1, quote);
        escaped += inner;
        escaped.push_back(quote);
        return escaped;
      }
      return expression.text;
    }
    case ExprKind::kIdentifier: {
      const bool is_local = local_types_.count(expression.text) != 0;
      const bool is_parameter = current_parameters_.count(expression.text) != 0;
      if (!is_local && !is_parameter && renames_.count(expression.text) == 0 &&
          MemberTypeOf(klass, expression.text).empty()) {
        const ClassInfo* klass_ref = symbols_.Resolve(expression.text, &klass);
        if (klass_ref != nullptr) {
          const std::string java_name = names_.ClassName(*klass_ref);
          return inline_substitution_ ? java_name : java_name + ".class";
        }
      }

      if (klass.declaration != nullptr) {
        for (const Member& member : klass.declaration->members) {
          if (member.kind == MemberKind::kConstant && member.name == expression.text &&
              member.expression_body != nullptr) {
            return EmitExpression(klass, *member.expression_body);
          }
        }
      }
      if (!is_local && !is_parameter) {
        const ClassInfo* global_owner = nullptr;
        if (const Member* global_member =
                FindGlobalClassMember(symbols_, klass, expression.text, &global_owner)) {
          if (global_owner != &klass)
            return names_.MemberName(*global_member);
        }
      }
      const auto rename = renames_.find(expression.text);
      return rename == renames_.end() ? expression.text : rename->second;
    }
    case ExprKind::kThis:
      return names_.ClassName(klass) + ".this";
    case ExprKind::kSuper:
      return "super";
    case ExprKind::kNull:
      return "null";
    case ExprKind::kFieldAccess: {
      if (expression.children.empty() || expression.children[0] == nullptr) {
        return expression.text;
      }

      if (expression.text == "长度") {
        const std::string receiver_type = StaticTypeNameOf(klass, *expression.children[0]);
        if (!receiver_type.empty() && receiver_type.back() == ']') {
          return EmitExpression(klass, *expression.children[0]) + ".length";
        }
      }

      if (expression.children[0]->kind == ExprKind::kIdentifier) {
        const ClassInfo* static_class = symbols_.Resolve(expression.children[0]->text, &klass);
        const bool shadowed = MemberTypeOf(klass, expression.children[0]->text).size() > 0 ||
                              local_types_.count(expression.children[0]->text) != 0 ||
                              current_parameters_.count(expression.children[0]->text) != 0;
        if (static_class != nullptr && !shadowed) {
          std::string field_name = expression.text;
          const ClassInfo* field_owner = nullptr;
          if (const Member* field =
                  FindMemberInChain(symbols_, *static_class, expression.text, &field_owner)) {
            field_name = names_.MemberName(*field);
          }
          return names_.ClassName(*static_class) + "." + field_name;
        }
      }

      const ClassInfo* receiver_class = ReceiverClass(klass, *expression.children[0]);
      if (receiver_class != nullptr && receiver_class->declaration != nullptr) {
        const ClassInfo* property_owner = nullptr;
        const Member* property = nullptr;
        const ClassInfo* cursor = receiver_class;
        std::set<const ClassInfo*> visited;
        while (cursor != nullptr && cursor->declaration != nullptr &&
               visited.insert(cursor).second) {
          for (const Member& member : cursor->declaration->members) {
            if (member.kind == MemberKind::kPropertyRead && member.name == expression.text) {
              property = &member;
              property_owner = cursor;
              break;
            }
          }
          if (property != nullptr || cursor->declaration->base_name.empty())
            break;
          cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
        }
        if (property != nullptr && property_owner != nullptr) {
          const std::string receiver = EmitExpression(klass, *expression.children[0]);
          if (property_owner->HasExternalType("java")) {
            return OwnerCallName(klass, *property_owner, expression.children[0].get()) + "." +
                   names_.MemberName(*property) + "(" + receiver + ")";
          }
          return receiver + "." + names_.MemberName(*property) + "()";
        }
      }
      if (expression.children[0]->kind == ExprKind::kIdentifier &&
          (expression.children[0]->text == "本对象" || expression.children[0]->text == "父对象")) {
        const ClassInfo* member_owner = nullptr;
        if (const Member* member =
                FindMemberInChain(symbols_, klass, expression.text, &member_owner)) {
          return names_.ClassName(klass) + ".this." + names_.MemberName(*member);
        }
      }
      if (expression.children[0]->kind == ExprKind::kThis) {
        const ClassInfo* member_owner = nullptr;
        if (const Member* member =
                FindMemberInChain(symbols_, klass, expression.text, &member_owner)) {
          return EmitExpression(klass, *expression.children[0]) + "." + names_.MemberName(*member);
        }
      }
      const ClassInfo* receiver_class2 = ReceiverClass(klass, *expression.children[0]);
      if (receiver_class2 != nullptr) {
        const ClassInfo* member_owner = nullptr;
        if (const Member* member =
                FindMemberInChain(symbols_, *receiver_class2, expression.text, &member_owner)) {
          return EmitExpression(klass, *expression.children[0]) + "." + names_.MemberName(*member);
        }
      }
      return EmitExpression(klass, *expression.children[0]) + "." + expression.text;
    }
    case ExprKind::kCall: {
      std::string inlined;
      if (TryInlineCode(klass, expression, &inlined))
        return inlined;
      if (expression.children.empty() || expression.children[0] == nullptr)
        return {};
      const Expr& callee = *expression.children[0];
      const ClassInfo* owner = nullptr;
      const Member* member = ResolveCalledMember(klass, callee, &owner);
      if (member != nullptr && owner != nullptr && owner->HasExternalType("java")) {
        std::string receiver;
        const Expr* receiver_expr = nullptr;
        if (callee.kind == ExprKind::kIdentifier) {
          receiver = alias_static_context_ ? "__this_obj" : names_.ClassName(klass) + ".this";
        } else {
          receiver_expr = callee.children[0].get();
          receiver = EmitExpression(klass, *callee.children[0]);
        }
        const std::string arguments = CallArguments(klass, *owner, *member, expression, false);
        std::string result = OwnerCallName(klass, *owner, receiver_expr) + "." +
                             names_.MemberName(*member) + "(" + receiver;
        if (!arguments.empty())
          result += ", " + arguments;
        result += ")";
        return result;
      }
      if (member != nullptr && owner != nullptr) {
        std::string receiver;
        if (callee.kind == ExprKind::kIdentifier) {
          receiver =
              (owner != &klass && owner->is_global_class) ? names_.ClassName(*owner) + "." : "";
        } else if (callee.children[0] != nullptr &&
                   callee.children[0]->kind == ExprKind::kIdentifier) {
          const ClassInfo* static_receiver = symbols_.Resolve(callee.children[0]->text, &klass);
          const bool shadowed = MemberTypeOf(klass, callee.children[0]->text).size() > 0 ||
                                local_types_.count(callee.children[0]->text) != 0 ||
                                current_parameters_.count(callee.children[0]->text) != 0;
          if (static_receiver != nullptr && !shadowed) {
            receiver = names_.ClassName(*static_receiver) + ".";
          } else {
            receiver = EmitExpression(klass, *callee.children[0]) + ".";
          }
        } else {
          receiver = EmitExpression(klass, *callee.children[0]) + ".";
        }
        return receiver + names_.MemberName(*member) + "(" +
               CallArguments(klass, *owner, *member, expression, false) + ")";
      }
      std::string result = EmitExpression(klass, callee) + "(";
      for (std::size_t i = 1; i < expression.children.size(); ++i) {
        if (i > 1)
          result += ", ";
        result += EmitInlineArgument(klass, expression.children[i].get());
      }
      result += ")";
      return result;
    }
    case ExprKind::kNew: {
      const ClassInfo* created = symbols_.Resolve(expression.text, &klass);
      const std::string type = JavaTypeOf(klass, expression.text);
      if (created != nullptr && IsComponentClass(*created)) {
        return "new " + type + "(" + names_.ClassName(klass) + ".this)";
      }
      return "new " + type + "()";
    }
    case ExprKind::kBinary: {
      long long folded = 0;
      if (TryFoldInteger(expression, &folded))
        return std::to_string(folded);
      std::string folded_text;
      if (expression.text == "+" && FoldStringConcat(expression, &folded_text)) {
        std::string escaped;
        for (char c : folded_text) {
          if (c == '\\')
            escaped += "\\\\";
          else if (c == '"')
            escaped += "\\\"";
          else
            escaped.push_back(c);
        }
        return "\"" + escaped + "\"";
      }
      if (expression.children.size() == 2 && expression.children[0] != nullptr &&
          expression.children[1] != nullptr) {
        const std::string left_type = StaticTypeNameOf(klass, *expression.children[0]);
        const ClassInfo* left_class = symbols_.Resolve(left_type, &klass);
        if (left_class != nullptr) {
          const ClassInfo* overload_owner = nullptr;
          if (const Member* overload =
                  FindMemberInChain(symbols_, *left_class, expression.text, &overload_owner)) {
            if (overload->kind == MemberKind::kMethod) {
              const Expr& left = *expression.children[0];
              const Expr& right = *expression.children[1];
              if (left_class->HasExternalType("java")) {
                return OwnerCallName(klass, *left_class, &left) + "." +
                       names_.MemberName(*overload) + "(" + EmitExpression(klass, left) + ", " +
                       EmitExpression(klass, right) + ")";
              }
              return EmitExpression(klass, left) + "." + names_.MemberName(*overload) + "(" +
                     EmitExpression(klass, right) + ")";
            }
          }
        }
      }
      std::string op = expression.text;
      if (op == "且")
        op = "&&";
      else if (op == "或")
        op = "||";
      const bool concatenation = op == "+";

      if (concatenation)
        ++plus_left_escape_passes_;
      std::string left = expression.children.size() > 0 && expression.children[0] != nullptr
                             ? EmitExpression(klass, *expression.children[0])
                             : "";
      if (concatenation)
        --plus_left_escape_passes_;
      std::string right = expression.children.size() > 1 && expression.children[1] != nullptr
                              ? EmitExpression(klass, *expression.children[1])
                              : "";
      return left + " " + op + " " + right;
    }
    case ExprKind::kUnary: {
      const std::string operand = expression.children.empty() || expression.children[0] == nullptr
                                      ? std::string()
                                      : EmitExpression(klass, *expression.children[0]);
      return expression.text + operand;
    }
    case ExprKind::kCast: {
      const std::string value = expression.children.empty() || expression.children[0] == nullptr
                                    ? std::string()
                                    : EmitExpression(klass, *expression.children[0]);
      return "(" + JavaTypeOf(klass, expression.text) + ") " + value;
    }
    case ExprKind::kParen: {
      return "(" +
             (expression.children.empty() || expression.children[0] == nullptr
                  ? std::string()
                  : EmitExpression(klass, *expression.children[0])) +
             ")";
    }
    case ExprKind::kArrayLiteral: {
      std::string result = "{";
      for (std::size_t i = 0; i < expression.children.size(); ++i) {
        if (i > 0)
          result += ", ";
        if (expression.children[i] != nullptr)
          result += EmitExpression(klass, *expression.children[i]);
      }
      result += "}";
      return result;
    }
    case ExprKind::kAssign: {
      if (expression.children.size() < 2 || expression.children[0] == nullptr ||
          expression.children[1] == nullptr) {
        return {};
      }
      std::string property_write;
      if (TryEmitPropertyWrite(klass, *expression.children[0], *expression.children[1],
                               &property_write)) {
        return property_write;
      }
      if (expression.children[0]->kind == ExprKind::kArrayAccess &&
          expression.children[0]->children.size() >= 2 &&
          expression.children[0]->children[0] != nullptr) {
        const Expr& access = *expression.children[0];
        const std::string target_type = StaticTypeNameOf(klass, *access.children[0]);
        if (!target_type.empty() && target_type.find('[') == std::string::npos) {
          const ClassInfo* target_class = symbols_.Resolve(BaseResolvableName(target_type), &klass);
          if (target_class != nullptr) {
            const ClassInfo* owner = nullptr;
            if (const Member* indexer = FindMemberInChain(symbols_, *target_class, "[]=", &owner)) {
              std::string value_type;
              if (indexer->parameters.size() >= 2)
                value_type = indexer->parameters[1].type_name;
              const std::string receiver = EmitExpression(klass, *access.children[0]);
              const std::string index = EmitExpression(klass, *access.children[1]);
              const std::string value = EmitConverted(klass, value_type, *expression.children[1]);
              if (owner != nullptr && owner->HasExternalType("java")) {
                return OwnerCallName(klass, *owner, access.children[0].get()) + "." +
                       names_.MemberName(*indexer) + "(" + receiver + ", " + index + ", " + value +
                       ")";
              }
              return receiver + "." + names_.MemberName(*indexer) + "(" + index + ", " + value +
                     ")";
            }
          }
        }
      }
      const std::string target_type = StaticTypeNameOf(klass, *expression.children[0]);
      return EmitExpression(klass, *expression.children[0]) + " = " +
             EmitConverted(klass, target_type, *expression.children[1]);
    }
    case ExprKind::kArrayAccess: {
      if (expression.children.size() < 2 || expression.children[0] == nullptr ||
          expression.children[1] == nullptr) {
        return {};
      }
      const std::string target_type = StaticTypeNameOf(klass, *expression.children[0]);
      if (!target_type.empty() && target_type.find('[') == std::string::npos) {
        const ClassInfo* target_class = symbols_.Resolve(BaseResolvableName(target_type), &klass);
        if (target_class != nullptr) {
          const ClassInfo* owner = nullptr;
          if (const Member* indexer = FindMemberInChain(symbols_, *target_class, "[]", &owner)) {
            const std::string receiver = EmitExpression(klass, *expression.children[0]);
            const std::string index = EmitExpression(klass, *expression.children[1]);
            if (owner != nullptr && owner->HasExternalType("java")) {
              return OwnerCallName(klass, *owner, expression.children[0].get()) + "." +
                     names_.MemberName(*indexer) + "(" + receiver + ", " + index + ")";
            }
            return receiver + "." + names_.MemberName(*indexer) + "(" + index + ")";
          }
        }
      }
      return EmitExpression(klass, *expression.children[0]) + "[" +
             EmitExpression(klass, *expression.children[1]) + "]";
    }
    case ExprKind::kIsType: {
      const std::string value = expression.children.empty() || expression.children[0] == nullptr
                                    ? std::string()
                                    : EmitExpression(klass, *expression.children[0]);
      return value + " instanceof " + types_.MapType(expression.text, &klass);
    }
    case ExprKind::kMulti: {
      std::string result;
      for (std::size_t i = 0; i < expression.children.size(); ++i) {
        if (i > 0)
          result += ", ";
        if (expression.children[i] != nullptr) {
          result += EmitExpression(klass, *expression.children[i]);
        }
      }
      return result;
    }
    case ExprKind::kAwait: {
      const auto replacement = await_replacements_.find(&expression);
      if (replacement != await_replacements_.end())
        return replacement->second;
      ReportUnsupported(klass, expression.position, "暂不支持的表达式: 等待", "codegen.await");
      return {};
    }
    case ExprKind::kExchange:
    case ExprKind::kUnknown:
      break;
  }
  return {};
}

std::string JavaEmitter::EmitInlineArgument(const ClassInfo& klass, const Expr* argument) {
  if (argument == nullptr)
    return {};
  return EmitExpression(klass, *argument);
}

std::string JavaEmitter::SubstituteCode(const ClassInfo& klass, const std::string& code,
                                        const std::map<std::string, std::string>& parameters) {
  std::string result;
  result.reserve(code.size());

  std::vector<std::pair<std::string, std::string>> ordered(parameters.begin(), parameters.end());
  std::sort(ordered.begin(), ordered.end(),
            [](const auto& a, const auto& b) { return a.first.size() > b.first.size(); });

  std::size_t i = 0;
  char quote = 0;
  while (i < code.size()) {
    const char current = code[i];
    if (quote != 0) {
      result.push_back(current);
      ++i;
      if (current == '\\' && i < code.size()) {
        result.push_back(code[i++]);
      } else if (current == quote) {
        quote = 0;
      }
      continue;
    }
    if (current == '"' || current == '\'') {
      quote = current;
      result.push_back(current);
      ++i;
      continue;
    }
    if (current != '#') {
      result.push_back(code[i++]);
      continue;
    }
    const std::size_t name_begin = i + 1;
    std::size_t name_end = name_begin;
    while (name_end < code.size() && IsIdentifierByte(code[name_end]))
      ++name_end;
    const std::string name = code.substr(name_begin, name_end - name_begin);

    static const char* kKeywords[] = {"cls", "ncls", "mem", "expr", "get", "set", "sys"};
    bool handled = false;
    for (const char* keyword : kKeywords) {
      if (name != keyword || name_end >= code.size() || code[name_end] != '<')
        continue;
      std::size_t index = name_end;
      std::string content;
      if (!ReadAngleContent(code, &index, &content))
        continue;
      const std::string key(keyword);
      if (key == "cls") {
        const ClassInfo* target = symbols_.Resolve(content, &klass);
        result += target == nullptr ? content : names_.ClassName(*target);
      } else if (key == "ncls") {
        result += types_.MapType(content, &klass);
      } else if (key == "mem") {
        const std::size_t dot = content.rfind('.');
        if (dot == std::string::npos) {
          result += content;
        } else {
          const ClassInfo* owner = symbols_.Resolve(content.substr(0, dot), &klass);
          std::string member_name = content.substr(dot + 1);
          const ClassInfo* member_owner = nullptr;
          if (owner != nullptr) {
            if (const Member* member =
                    FindMemberInChain(symbols_, *owner, member_name, &member_owner)) {
              member_name = names_.MemberName(*member);
            }
          }
          result += owner == nullptr ? content : names_.ClassName(*owner) + "." + member_name;
        }
      } else if (key == "expr") {
        result += TranslateExpressionNames(klass, content);
      } else if (key == "get" || key == "set") {
        const MemberKind wanted =
            key == "get" ? MemberKind::kPropertyRead : MemberKind::kPropertyWrite;
        const Member* property = nullptr;
        const ClassInfo* cursor = &klass;
        std::set<const ClassInfo*> visited;
        while (cursor != nullptr && cursor->declaration != nullptr &&
               visited.insert(cursor).second) {
          for (const Member& member : cursor->declaration->members) {
            if (member.kind == wanted && member.name == content)
              property = &member;
          }
          if (property != nullptr || cursor->declaration->base_name.empty())
            break;
          cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
        }
        result += property == nullptr ? content : names_.MemberName(*property);
      } else {
        if (content == "main_pkg") {
          const std::string resource_namespace = platform_.ResourceNamespace();
          result += resource_namespace.empty() ? config_.namespace_name : resource_namespace;
        } else if (content == "source") {
          result += klass.declaration != nullptr ? klass.declaration->file : std::string();
        } else if (content == "line") {
          const int line = current_code_line_ > 0
                               ? current_code_line_
                               : (klass.declaration != nullptr
                                      ? static_cast<int>(klass.declaration->position.line) + 1
                                      : 0);
          result += std::to_string(line);
        } else if (content == "time") {
          const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::system_clock::now().time_since_epoch())
                                        .count();
          result += std::to_string(milliseconds) + "L";
        }
      }
      i = index;
      handled = true;
      break;
    }
    if (handled)
      continue;

    if (name == "this" && (name_end >= code.size() || code[name_end] != '<')) {
      result += alias_static_context_ ? "__this_obj" : names_.ClassName(klass) + ".this";
      i = name_end;
      continue;
    }

    bool substituted = false;
    for (const auto& entry : ordered) {
      if (entry.first != name)
        continue;
      result += entry.second;
      substituted = true;
      break;
    }
    if (!substituted) {
      const auto renamed = renames_.find(name);
      if (renamed != renames_.end()) {
        result += renamed->second;
        substituted = true;
      }
    }
    if (!substituted) {
      const ClassInfo* owner = nullptr;
      if (const Member* member = FindMemberInChain(symbols_, klass, name, &owner)) {
        result += names_.MemberName(*member);
        substituted = true;
      }
    }
    if (substituted) {
      i = name_end;
      continue;
    }
    result.push_back('#');
    ++i;
  }
  return result;
}

std::string JavaEmitter::EmitArrayLiteral(const ClassInfo& klass, const Expr& expression,
                                          const std::string& element_type, int dimensions) {
  const int dims = dimensions > 0 ? dimensions : 1;
  std::string result = "new " + element_type;
  for (int i = 0; i < dims; ++i)
    result += "[]";
  result += "{";
  for (std::size_t i = 0; i < expression.children.size(); ++i) {
    if (i > 0)
      result += ", ";
    const Expr* child = expression.children[i].get();
    if (child == nullptr)
      continue;
    if (dims > 1 && child->kind == ExprKind::kArrayLiteral) {
      result += EmitArrayLiteral(klass, *child, element_type, dims - 1);
    } else {
      result += EmitExpression(klass, *child);
    }
  }
  result += "}";
  return result;
}

void JavaEmitter::BuildInlineTargetIndex() {
  inline_targets_.clear();
  const auto collect = [this](bool global_classes) {
    for (const auto& entry : symbols_.classes()) {
      const ClassInfo& candidate = entry.second;
      if (candidate.is_global_class != global_classes || candidate.declaration == nullptr)
        continue;
      std::set<std::string> indexed;
      for (const Member& member : candidate.declaration->members) {
        if (CodeTextOf(member).empty())
          continue;
        if (effects_.Get<JavaInlineCodeSpec>(&member) == nullptr)
          continue;
        if (!indexed.insert(member.name).second)
          continue;
        inline_targets_[member.name].push_back(InlineTarget{&candidate, &member});
      }
    }
  };
  collect(true);
  collect(false);
  inline_targets_ready_ = true;
}

bool JavaEmitter::TryInlineCode(const ClassInfo& klass, const Expr& call, std::string* out_text,
                                LineRef* source_ref) {
  if (call.kind != ExprKind::kCall || call.children.empty() || call.children[0] == nullptr) {
    return false;
  }
  if (call.children[0]->kind != ExprKind::kIdentifier)
    return false;
  const std::string& method_name = call.children[0]->text;

  const Member* target = nullptr;
  const ClassInfo* owner = nullptr;
  if (klass.declaration != nullptr) {
    for (const Member& member : klass.declaration->members) {
      if (member.name != method_name)
        continue;
      if (CodeTextOf(member).empty())
        continue;
      if (effects_.Get<JavaInlineCodeSpec>(&member) != nullptr) {
        target = &member;
        owner = &klass;
        break;
      }
    }
  }
  if (target == nullptr) {
    if (!inline_targets_ready_)
      BuildInlineTargetIndex();
    const auto it = inline_targets_.find(method_name);
    if (it != inline_targets_.end() && !it->second.empty()) {
      target = it->second.front().member;
      owner = it->second.front().owner;
    }
  }
  if (target == nullptr || owner == nullptr)
    return false;

  std::map<std::string, std::string> substitutions;
  inline_substitution_ = true;
  for (std::size_t i = 0; i < target->parameters.size(); ++i) {
    const std::size_t argument_index = i + 1;
    if (argument_index < call.children.size()) {
      substitutions[target->parameters[i].name] =
          EmitInlineArgument(klass, call.children[argument_index].get());
    } else if (target->parameters[i].default_value != nullptr) {
      substitutions[target->parameters[i].name] =
          EmitExpression(*owner, *target->parameters[i].default_value);
    }
  }
  inline_substitution_ = false;
  inline_code_indent_ = CodeIndentOf(*target);
  current_code_line_ = CodeLineOf(*target);
  *out_text = SubstituteCode(klass, CodeTextOf(*target), substitutions);
  current_code_line_ = 0;
  if (source_ref != nullptr) {
    source_ref->file = owner->declaration != nullptr ? owner->declaration->file : std::string();
    source_ref->line = CodeLineOf(*target);
  }
  return true;
}

}
