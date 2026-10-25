#include "constants.h"
#include "java_emitter.h"

#include <algorithm>
#include <set>
#include <string>

#include "internal/helpers.h"
#include "symbol_resolution.h"
#include "text_util.h"

namespace tiecodec {

using namespace detail;
namespace {

std::string BaseTypeName(const std::string& type_name) {
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
    if (c == '[')
      break;
    if (c == '?')
      break;
    result.push_back(c);
  }
  return result;
}

}

std::string JavaEmitter::MemberCodeAnnotation(const Member& member, const std::string& name) const {
  for (const Annotation& annotation : member.annotations) {
    if (annotation.name != name)
      continue;
    return annotation.TextArgument(kArgumentParameter);
  }
  return {};
}

std::string JavaEmitter::StaticTypeNameOf(const ClassInfo& klass, const Expr& expression) const {
  switch (expression.kind) {
    case ExprKind::kIdentifier: {
      const auto local = local_types_.find(expression.text);
      if (local != local_types_.end())
        return local->second;
      for (const Parameter& parameter : current_parameter_types_) {
        if (parameter.name == expression.text)
          return parameter.type_name;
      }
      return MemberTypeOf(klass, expression.text);
    }
    case ExprKind::kLiteral: {
      if (expression.text == "真" || expression.text == "假")
        return builtin_type::kBoolean;
      if (!expression.raw.empty() && (expression.raw[0] == '"' || expression.raw[0] == '[')) {
        return builtin_type::kText;
      }
      if (!expression.raw.empty() && expression.raw[0] == '\'')
        return builtin_type::kCharacter;
      if (expression.text.find('.') != std::string::npos)
        return builtin_type::kDecimal;
      return builtin_type::kInteger;
    }
    case ExprKind::kFieldAccess: {
      if (expression.children.empty() || expression.children[0] == nullptr)
        return {};
      const ClassInfo* receiver = ReceiverClass(klass, *expression.children[0]);
      if (receiver == nullptr)
        return {};
      const ClassInfo* owner = nullptr;
      if (const Member* member = FindMemberInChain(symbols_, *receiver, expression.text, &owner)) {
        return member->type_name;
      }
      return {};
    }
    case ExprKind::kBinary: {
      if (expression.children.size() != 2 || expression.children[0] == nullptr)
        return {};
      const std::string left_type = StaticTypeNameOf(klass, *expression.children[0]);
      const ClassInfo* left_class = symbols_.Resolve(left_type, &klass);
      if (left_class == nullptr)
        return {};
      const ClassInfo* owner = nullptr;
      if (const Member* overload =
              FindMemberInChain(symbols_, *left_class, expression.text, &owner)) {
        return overload->type_name;
      }
      return {};
    }
    case ExprKind::kParen: {
      if (expression.children.empty() || expression.children[0] == nullptr)
        return {};
      return StaticTypeNameOf(klass, *expression.children[0]);
    }
    default:
      return {};
  }
}

std::string JavaEmitter::LValueJavaType(const ClassInfo& klass, const Expr& target) const {
  switch (target.kind) {
    case ExprKind::kIdentifier: {
      const std::string type = StaticTypeNameOf(klass, target);
      return type.empty() ? "Object" : types_.MapType(type, &klass);
    }
    case ExprKind::kFieldAccess: {
      const std::string type = StaticTypeNameOf(klass, target);
      return type.empty() ? "Object" : types_.MapType(type, &klass);
    }
    case ExprKind::kArrayAccess: {
      if (target.children.empty() || target.children[0] == nullptr)
        return "Object";
      const std::string type = StaticTypeNameOf(klass, *target.children[0]);
      const TypeShape shape = ParseTypeShape(type);
      if (shape.base.empty() || type.find('[') != std::string::npos)
        return types_.MapType(shape.base, &klass);
      const ClassInfo* receiver = symbols_.Resolve(BaseResolvableName(shape.base), &klass);
      if (receiver != nullptr) {
        const ClassInfo* owner = nullptr;
        if (const Member* indexer = FindMemberInChain(symbols_, *receiver, "[]", &owner)) {
          return indexer->type_name.empty() ? "Object" : types_.MapType(indexer->type_name, &klass);
        }
      }
      return "Object";
    }
    default:
      return "Object";
  }
}

void JavaEmitter::EmitSwap(const ClassInfo& klass, const Expr& exchange, int indent,
                           EmittedText* out) {
  if (exchange.children.size() < 2 || exchange.children[0] == nullptr ||
      exchange.children[1] == nullptr) {
    return;
  }
  const Expr& left = *exchange.children[0];
  const Expr& right = *exchange.children[1];
  const std::string prefix =
      IsOriginalNaming(config_.friendly_name) ? kRandomNamePrefix : kRenamedTempPrefix;
  const std::string temp = names_.GeneratedName(prefix, ++generated_name_counter_);
  const auto object_index_writer = [&](const Expr& operand) -> const Member* {
    if (operand.kind != ExprKind::kArrayAccess || operand.children.size() < 2 ||
        operand.children[0] == nullptr) {
      return nullptr;
    }
    const std::string type = StaticTypeNameOf(klass, *operand.children[0]);
    if (type.empty() || type.find('[') != std::string::npos)
      return nullptr;
    const ClassInfo* receiver = symbols_.Resolve(BaseResolvableName(type), &klass);
    if (receiver == nullptr)
      return nullptr;
    const ClassInfo* owner = nullptr;
    return FindMemberInChain(symbols_, *receiver, kOperatorIndexAssign, &owner);
  };
  const Member* right_writer = object_index_writer(right);
  if (right_writer != nullptr) {
    const auto raw_index_text = [&](const Expr& operand) -> std::string {
      if (operand.kind == ExprKind::kArrayAccess && operand.children.size() >= 2 &&
          operand.children[0] != nullptr && operand.children[1] != nullptr) {
        return EmitExpression(klass, *operand.children[0]) + "[" +
               EmitExpression(klass, *operand.children[1]) + "]";
      }
      return EmitExpression(klass, operand);
    };
    const std::string receiver_text = EmitExpression(klass, *right.children[0]);
    const std::string index_text = EmitExpression(klass, *right.children[1]);
    const std::string writer_name = names_.MemberName(*right_writer);
    Indent(indent, out);
    out->append(LValueJavaType(klass, right) + " " + temp + " = " + EmitExpression(klass, right) +
                ";\n");
    Indent(indent, out);
    out->append(receiver_text + "." + writer_name + "(" + index_text + ", " + raw_index_text(left) +
                ");\n");
    Indent(indent, out);
    out->append(receiver_text + "." + writer_name + "(" + index_text + ", " + temp + ");\n");
    return;
  }
  const std::string type = LValueJavaType(klass, left);
  Indent(indent, out);
  out->append(type + " " + temp + " = " + EmitExpression(klass, right) + ";\n");
  Indent(indent, out);
  out->append(EmitExpression(klass, right) + " = " + EmitExpression(klass, left) + ";\n");
  Indent(indent, out);
  out->append(EmitExpression(klass, left) + " = " + temp + ";\n");
}

bool JavaEmitter::HasAssignmentOperator(const ClassInfo& klass,
                                        const std::string& type_name) const {
  const ClassInfo* target = symbols_.Resolve(type_name, &klass);
  if (target == nullptr)
    return false;
  const ClassInfo* owner = nullptr;
  const Member* member = FindMemberInChain(symbols_, *target, "=", &owner);
  if (member == nullptr || member->kind != MemberKind::kMethod)
    return false;
  if (std::any_of(member->annotations.begin(), member->annotations.end(),
                  [](const Annotation& annotation) { return annotation.name == "运算符重载"; })) {
    return true;
  }
  const std::string& name = member->name;
  return name == "=" || name == "+" || name == "-" || name == "*" || name == "/" || name == "%" ||
         name == "==" || name == "!=" || name == "<" || name == "<=" || name == ">" ||
         name == ">=" || name == "&" || name == "|" || name == "^";
}

const Member* JavaEmitter::FindImplicitCast(const ClassInfo& klass, const std::string& target_type,
                                            const Expr& value) const {
  const std::string source_type = StaticTypeNameOf(klass, value);
  const ClassInfo* source = symbols_.Resolve(source_type, &klass);
  const ClassInfo* target = symbols_.Resolve(target_type, &klass);
  if (source == nullptr || target == nullptr || source == target)
    return nullptr;
  const ClassInfo* cursor = source;
  std::set<const ClassInfo*> visited;
  while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
    for (const Member& member : cursor->declaration->members) {
      std::string operation;
      for (const Annotation& annotation : member.annotations) {
        if (annotation.name != "隐式操作")
          continue;
        operation = annotation.TextArgument(kArgumentParameter);
      }
      if (operation != "cast")
        continue;
      if (!member.parameters.empty())
        continue;
      const ClassInfo* cast_target = symbols_.Resolve(member.type_name, &klass);
      if (cast_target == nullptr)
        continue;
      const bool matches = cast_target == target;
      const bool assignable = [&]() {
        const ClassInfo* walk = target;
        std::set<const ClassInfo*> base_visited;
        while (walk != nullptr && walk->declaration != nullptr &&
               base_visited.insert(walk).second) {
          if (walk == cast_target)
            return true;
          if (walk->declaration->base_name.empty())
            break;
          walk = symbols_.Resolve(walk->declaration->base_name, walk);
        }
        return false;
      }();
      if (matches || assignable)
        return &member;
    }
    if (cursor->declaration->base_name.empty())
      break;
    cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
  }
  return nullptr;
}

std::string JavaEmitter::EmitConverted(const ClassInfo& klass, const std::string& target_type,
                                       const Expr& value) {
  if (target_type.empty())
    return EmitExpression(klass, value);
  return InitializerExpression(klass, target_type, value);
}

std::string JavaEmitter::InitializerExpression(const ClassInfo& klass,
                                               const std::string& declared_type,
                                               const Expr& value) {
  if (value.kind == ExprKind::kNull) {
    const std::string mapped = types_.MapType(SubstituteGenericType(declared_type), &klass);
    if (mapped == "int")
      return "0";
    if (mapped == "long")
      return "0L";
    if (mapped == "float")
      return "0.0F";
    if (mapped == "double")
      return "0.0";
    if (mapped == "boolean")
      return "false";
    if (mapped == "byte")
      return "(byte) 0";
    if (mapped == "short")
      return "(short) 0";
    if (mapped == "char")
      return "'\\0'";
    return "null";
  }
  const TypeShape shape = ParseTypeShape(declared_type);
  if (shape.dimensions > 0 && value.kind == ExprKind::kArrayLiteral) {
    return EmitArrayLiteral(klass, value, types_.MapType(shape.base, &klass), shape.dimensions);
  }
  const bool direct_construction =
      (value.kind == ExprKind::kNew && BaseTypeName(value.text) == BaseTypeName(declared_type)) ||
      value.kind == ExprKind::kMulti;
  if (!direct_construction) {
    if (const Member* cast = FindImplicitCast(klass, declared_type, value)) {
      return EmitExpression(klass, value) + "." + names_.MemberName(*cast) + "()";
    }
  }
  if (value.kind == ExprKind::kIdentifier || value.kind == ExprKind::kFieldAccess) {
    const ClassInfo* declared = symbols_.Resolve(declared_type, &klass);
    const std::string value_type = StaticTypeNameOf(klass, value);
    const ClassInfo* value_class = symbols_.Resolve(value_type, &klass);
    const bool upcast = [&]() {
      if (declared == nullptr || value_class == nullptr)
        return false;
      const ClassInfo* cursor = value_class;
      std::set<const ClassInfo*> visited;
      while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
        if (cursor == declared)
          return true;
        if (cursor->declaration->base_name.empty())
          break;
        cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
      }
      return false;
    }();
    if (declared != nullptr && value_class != nullptr && declared != value_class && !upcast &&
        !HasAssignmentOperator(klass, declared_type)) {
      return "(" + types_.MapType(declared_type, &klass) + ")" + EmitExpression(klass, value);
    }
  }
  if (HasAssignmentOperator(klass, declared_type)) {
    const bool direct_creation =
        value.kind == ExprKind::kNew && BaseTypeName(value.text) == BaseTypeName(declared_type);
    const ClassInfo* declared = symbols_.Resolve(declared_type, &klass);
    const ClassInfo* value_class = symbols_.Resolve(StaticTypeNameOf(klass, value), &klass);
    const bool same_type = declared != nullptr && value_class == declared;
    if (!direct_creation && !same_type) {
      std::string receiver = declared_type;
      std::string assignment_name = "=";
      if (declared != nullptr)
        receiver = names_.ClassName(*declared);
      const ClassInfo* owner = nullptr;
      if (const Member* assignment = FindMemberInChain(symbols_, *declared, "=", &owner)) {
        assignment_name = names_.MemberName(*assignment);
      }
      return receiver + "." + assignment_name + "(" + EmitExpression(klass, value) + ")";
    }
  }
  return EmitExpression(klass, value);
}

std::string JavaEmitter::TranslateExpressionNames(const ClassInfo& klass, const std::string& code) {
  std::string result;
  result.reserve(code.size());
  char quote = 0;
  std::size_t index = 0;
  while (index < code.size()) {
    const char c = code[index];
    if (quote != 0) {
      result.push_back(c);
      if (c == '\\' && index + 1 < code.size()) {
        result.push_back(code[++index]);
      } else if (c == quote) {
        quote = 0;
      }
      ++index;
      continue;
    }
    if (c == '"' || c == '\'') {
      quote = c;
      result.push_back(c);
      ++index;
      continue;
    }
    const unsigned char lead = static_cast<unsigned char>(c);
    const bool identifier_start = lead >= 0x80 || std::isalpha(lead) != 0 || c == '_';
    if (!identifier_start) {
      result.push_back(c);
      ++index;
      continue;
    }
    const std::size_t begin = index;
    while (index < code.size()) {
      const unsigned char byte = static_cast<unsigned char>(code[index]);
      if (byte >= 0x80 || std::isalnum(byte) != 0 || code[index] == '_') {
        ++index;
        continue;
      }
      break;
    }
    const std::string run = code.substr(begin, index - begin);
    const auto renamed = renames_.find(run);
    if (renamed != renames_.end()) {
      result += renamed->second;
      continue;
    }
    const auto parameter = current_parameters_.find(run);
    if (parameter != current_parameters_.end()) {
      result += parameter->second;
      continue;
    }
    const ClassInfo* owner = nullptr;
    if (const Member* member = FindMemberInChain(symbols_, klass, run, &owner)) {
      result += names_.MemberName(*member);
      continue;
    }
    result += run;
  }
  return result;
}

bool JavaEmitter::FoldStringConcat(const Expr& expression, std::string* value) const {
  switch (expression.kind) {
    case ExprKind::kLiteral: {
      if (!expression.raw.empty() && (expression.raw[0] == '"' || expression.raw[0] == '[')) {
        *value = expression.raw[0] == '"' ? expression.text : expression.text;
        return true;
      }
      if (expression.text == "真" || expression.text == "假") {
        *value = expression.text == "真" ? "true" : "false";
        return true;
      }
      *value = expression.text;
      return true;
    }
    case ExprKind::kParen: {
      if (expression.children.size() != 1 || expression.children[0] == nullptr)
        return false;
      return FoldStringConcat(*expression.children[0], value);
    }
    case ExprKind::kBinary: {
      if (expression.text != "+" || expression.children.size() != 2 ||
          expression.children[0] == nullptr || expression.children[1] == nullptr) {
        return false;
      }
      std::string left;
      std::string right;
      if (!FoldStringConcat(*expression.children[0], &left))
        return false;
      if (!FoldStringConcat(*expression.children[1], &right))
        return false;
      *value = left + right;
      return true;
    }
    default:
      return false;
  }
}

}
