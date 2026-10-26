#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "cxx_emitter.h"
#include "cxx_language_constants.h"
#include "internal/helpers.h"
#include "symbol_resolution.h"
#include "text_util.h"

namespace tiecodec {
namespace {

bool IsNumericBase(const std::string& base) {
  return base == "整数" || base == "长整数" || base == "小数" || base == "单精度小数" ||
         base == "字节" || base == "字符" || base == "逻辑型";
}

bool IsTextBase(const std::string& base) {
  return base == "文本";
}

bool IsObjectBase(const std::string& base) {
  return base == "对象";
}

std::string StripTrailingSemicolon(std::string text) {
  if (text.find('\n') != std::string::npos)
    return text;
  const std::size_t end = text.find_last_not_of(" \t");
  if (end == std::string::npos)
    return std::string();
  if (text[end] == ';')
    return text.substr(0, end);
  return text;
}

std::string TrimTrailing(const std::string& text) {
  const std::size_t end = text.find_last_not_of(" \t\r\n");
  if (end == std::string::npos)
    return std::string();
  return text.substr(0, end + 1);
}

bool LooksNumericLiteral(const std::string& text) {
  if (text.empty())
    return false;
  char c = text.front();
  return (c >= '0' && c <= '9') ||
         (c == '-' && text.size() > 1 && text[1] >= '0' && text[1] <= '9');
}

std::string StripReturnPrefix(std::string text) {
  if (text.find('\n') != std::string::npos)
    return text;
  const std::size_t first = text.find_first_not_of(" \t");
  if (first == std::string::npos)
    return text;
  const std::string prefix = "return ";
  if (text.compare(first, prefix.size(), prefix) == 0)
    return text.substr(first + prefix.size());
  return text;
}

}

std::string CxxEmitter::StaticTypeOf(const ClassInfo& klass, const Expr& expression) const {
  switch (expression.kind) {
    case ExprKind::kLiteral: {
      const std::string& text = expression.raw.empty() ? expression.text : expression.raw;
      if (text.empty())
        return {};
      if (text.front() == '"')
        return "文本";
      if (text.front() == '\'')
        return "字符";
      if (text == "真" || text == "假")
        return "逻辑型";
      if (text == "空")
        return {};
      if (LooksNumericLiteral(text)) {
        if (text.back() == 'L' || text.back() == 'l')
          return "长整数";
        if (text.back() == 'F' || text.back() == 'f')
          return "单精度小数";
        if (text.find('.') != std::string::npos)
          return "小数";
        return "整数";
      }
      return {};
    }
    case ExprKind::kIdentifier: {
      std::string type;
      if (TryLocalType(expression.text, &type))
        return type;
      const CxxMemberInfo* info = LookupMember(klass, expression.text);
      if (info != nullptr && info->is_field)
        return info->type_name;
      return {};
    }
    case ExprKind::kThis:
      return klass.qualified_name;
    case ExprKind::kParen:
      if (expression.children.size() == 1 && expression.children[0] != nullptr)
        return StaticTypeOf(klass, *expression.children[0]);
      return {};
    case ExprKind::kCast:
      return expression.text;
    case ExprKind::kNew:
      return expression.text;
    case ExprKind::kFieldAccess: {
      if (expression.children.empty() || expression.children[0] == nullptr)
        return {};
      const std::string receiver_type = StaticTypeOf(klass, *expression.children[0]);
      const std::string base = CxxBaseTypeName(receiver_type);
      if (base.empty())
        return {};
      const ClassInfo* receiver_class = symbols_.Resolve(base, &klass);
      if (receiver_class == nullptr)
        return {};
      const ClassInfo* owner = nullptr;
      const Member* member = FindMemberInChain(symbols_, *receiver_class, expression.text, &owner);
      if (member != nullptr && member->kind == MemberKind::kPropertyWrite)
        member = FindPropertyAccessor(*receiver_class, expression.text, MemberKind::kPropertyRead);
      if (member == nullptr)
        return {};
      if (member->kind == MemberKind::kPropertyRead)
        return member->type_name;
      if (member->kind == MemberKind::kVariable || member->kind == MemberKind::kConstant)
        return member->type_name;
      return {};
    }
    case ExprKind::kArrayAccess: {
      if (expression.children.empty() || expression.children[0] == nullptr)
        return {};
      const std::string container = StaticTypeOf(klass, *expression.children[0]);
      return ElementTypeOf(klass, container);
    }
    case ExprKind::kUnary:
      if (expression.children.size() == 1 && expression.children[0] != nullptr)
        return StaticTypeOf(klass, *expression.children[0]);
      return {};
    case ExprKind::kBinary: {
      const std::string& op = expression.text;
      if (op == "且" || op == "或" || op == "&&" || op == "||" || op == "==" || op == "!=" ||
          op == "<" || op == ">" || op == "<=" || op == ">=")
        return "逻辑型";
      if (expression.children.size() != 2 || expression.children[0] == nullptr ||
          expression.children[1] == nullptr)
        return {};
      const std::string left = StaticTypeOf(klass, *expression.children[0]);
      const std::string right = StaticTypeOf(klass, *expression.children[1]);
      if (op == "+" && (left == "文本" || right == "文本"))
        return "文本";
      if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        if (IsNumericBase(left))
          return left;
        if (IsNumericBase(right))
          return right;
      }
      return {};
    }
    case ExprKind::kCall: {
      Expr call_copy = expression;
      const ClassInfo* receiver_class = nullptr;
      std::string receiver_type;
      const CxxMemberInfo* info =
          ResolveCalledMemberInner(call_copy, &receiver_class, &receiver_type);
      if (info != nullptr && info->member != nullptr)
        return info->member->type_name;
      return {};
    }
    default:
      return {};
  }
}

std::string CxxEmitter::EmitExpression(const ClassInfo& klass, const Expr& expression) {
  switch (expression.kind) {
    case ExprKind::kLiteral:
      return EmitLiteral(klass, expression, std::string());
    case ExprKind::kIdentifier:
      return EmitIdentifier(klass, expression);
    case ExprKind::kThis:
      if (inline_substitution_ && !inline_receiver_.empty())
        return inline_receiver_;
      return "this";
    case ExprKind::kNull:
      return kCxxNullLiteral;
    case ExprKind::kSuper:
      return context_.klass != nullptr ? cxx_names_.BaseName(*context_.klass) : std::string();
    case ExprKind::kFieldAccess:
      return EmitFieldAccess(klass, expression);
    case ExprKind::kCall: {
      const std::string inlined = TryInlineCode(klass, expression);
      if (!inlined.empty())
        return StripReturnPrefix(StripTrailingSemicolon(inlined));
      return EmitCall(klass, expression);
    }
    case ExprKind::kNew:
      return EmitNew(klass, expression);
    case ExprKind::kBinary:
      return EmitBinary(klass, expression);
    case ExprKind::kUnary: {
      const std::string operand = expression.children.empty() || expression.children[0] == nullptr
                                      ? std::string()
                                      : EmitExpression(klass, *expression.children[0]);
      if (expression.text == "!")
        return "!" + operand;
      return expression.text + operand;
    }
    case ExprKind::kCast: {
      if (expression.children.empty() || expression.children[0] == nullptr)
        return {};
      const std::string operand = EmitExpression(klass, *expression.children[0]);
      const std::string mapped = TypeName(klass, expression.text);
      if (mapped.rfind(kCxxSharedPtr, 0) == 0) {
        return "std::static_pointer_cast<" + mapped + ">(" + operand + ")";
      }
      return "static_cast<" + mapped + ">(" + operand + ")";
    }
    case ExprKind::kArrayLiteral:
      return EmitArrayLiteral(klass, expression, std::string());
    case ExprKind::kArrayAccess:
      return EmitArrayAccess(klass, expression);
    case ExprKind::kParen: {
      if (expression.children.empty() || expression.children[0] == nullptr)
        return "()";
      return "(" + EmitExpression(klass, *expression.children[0]) + ")";
    }
    case ExprKind::kMulti: {
      std::string joined;
      for (std::size_t i = 0; i < expression.children.size(); ++i) {
        if (i > 0)
          joined += ", ";
        if (expression.children[i] != nullptr)
          joined += EmitExpression(klass, *expression.children[i]);
      }
      return joined;
    }
    case ExprKind::kIsType: {
      if (expression.children.empty() || expression.children[0] == nullptr)
        return "false";
      const std::string value = EmitExpression(klass, *expression.children[0]);
      const ClassInfo* target = symbols_.Resolve(CxxBaseTypeName(expression.text), &klass);
      if (target == nullptr) {
        ReportUnsupported(klass, expression.position,
                          "C++ 目标的属于判断找不到类型 \"" + expression.text + "\"",
                          "codegen.unresolved-type");
        return "false";
      }
      if (ReferenceShape())
        ReportUnsupported(
            klass, expression.position,
            "禁用 FreeTiecodec 特性时，属于判断的参考形态（嵌套指针转换恒为空）是静默错误",
            "compat.ftc.is-type");
      std::string cast_argument;
      if (IsExternalClass(*target)) {
        if (target->declaration != nullptr && !target->declaration->generic_parameters.empty()) {
          ReportUnsupported(klass, expression.position,
                            "C++ 目标的属于判断暂不支持外部泛型类型 \"" + expression.text + "\"",
                            "codegen.unsupported-istype");
          return "false";
        }
        cast_argument = target->ExternalType(kCxxTypeAliasKey);
      } else {
        cast_argument = ClassValueName(*target);
      }
      return "std::dynamic_pointer_cast<" + cast_argument + ">(" + value + ") != nullptr";
    }
    case ExprKind::kAwait:
      ReportUnsupported(klass, expression.position, "C++ 目标暂不支持等待表达式",
                        "codegen.unsupported-await");
      return "false";
    case ExprKind::kAssign:
      return EmitAssign(klass, expression);
    case ExprKind::kExchange:
      ReportUnsupported(klass, expression.position, "C++ 目标暂不支持表达式位置的交换",
                        "codegen.unsupported-swap");
      return "false";
    case ExprKind::kUnknown:
    default:
      return expression.raw.empty() ? expression.text : expression.raw;
  }
}

std::string CxxEmitter::EmitIdentifier(const ClassInfo& klass, const Expr& expression) {
  const std::string& name = expression.text;
  const auto parameter = current_parameters_.find(name);
  if (parameter != current_parameters_.end())
    return parameter->second;
  std::string local_type;
  if (TryLocalType(name, &local_type))
    return name;
  const CxxMemberInfo* info = LookupMember(klass, name);
  if (info != nullptr) {
    if (info->is_static_member && info->owner != nullptr)
      return StaticOwnerRef(*info->owner) + "::" + cxx_names_.MemberName(*info->member);
    return cxx_names_.MemberName(*info->member);
  }
  const ClassInfo* target = symbols_.Resolve(name, &klass);
  if (target != nullptr)
    return ClassValueName(*target);
  const auto binding = generic_bindings_.find(name);
  if (binding != generic_bindings_.end())
    return binding->second;
  const std::vector<GlobalMemberRef>* refs = symbols_.GlobalMembers(name);
  if (refs != nullptr) {
    for (const GlobalMemberRef& ref : *refs) {
      if (ref.member == nullptr || ref.owner == nullptr)
        continue;
      if (ref.member->kind == MemberKind::kVariable || ref.member->kind == MemberKind::kConstant)
        return StaticOwnerRef(*ref.owner) + "::" + cxx_names_.MemberName(*ref.member);
    }
  }
  return name;
}

std::string CxxEmitter::EmitExpressionAs(const ClassInfo& klass, const Expr& expression,
                                         const std::string& expected_type) {
  if (expression.kind == ExprKind::kLiteral || expression.kind == ExprKind::kNull)
    return EmitConverted(klass, expected_type, expression);
  if (expression.kind == ExprKind::kParen && expression.children.size() == 1 &&
      expression.children[0] != nullptr &&
      (expression.children[0]->kind == ExprKind::kLiteral ||
       expression.children[0]->kind == ExprKind::kArrayLiteral)) {
    return EmitConverted(klass, expected_type, *expression.children[0]);
  }
  return EmitConverted(klass, expected_type, expression);
}

std::string CxxEmitter::BoxedExpression(const ClassInfo& klass, const std::string& source_type,
                                        const std::string& expression_text) const {
  const CxxMappedType mapped = cxx_types_.Describe(CxxBaseTypeName(source_type), &klass);
  const std::string wrapper = mapped.is_primitive ? mapped.wrapper : mapped.wrapper;
  if (wrapper.empty())
    return expression_text;
  return std::string(kCxxMakeShared) + "<" + wrapper + ">(" + expression_text + ")";
}

std::string CxxEmitter::EmitConvertValue(const ClassInfo& klass, const std::string& tie_type,
                                         const Expr& value) {
  if (tie_type.empty())
    return EmitConverted(klass, tie_type, value);
  const ClassInfo* target = symbols_.Resolve(CxxBaseTypeName(tie_type), &klass);
  if (target == nullptr)
    return EmitConverted(klass, tie_type, value);
  const ClassInfo* owner = nullptr;
  const Member* assign_op = FindMemberInChain(symbols_, *target, kOperatorAssign, &owner);
  if (assign_op == nullptr)
    return EmitConverted(klass, tie_type, value);
  ExprPtr receiver = std::make_shared<Expr>();
  receiver->kind = ExprKind::kIdentifier;
  receiver->text = CxxBaseTypeName(tie_type);
  receiver->raw = receiver->text;
  receiver->position = value.position;
  ExprPtr callee = std::make_shared<Expr>();
  callee->kind = ExprKind::kFieldAccess;
  callee->text = kOperatorAssign;
  callee->position = value.position;
  callee->children.push_back(receiver);
  ExprPtr synthetic = std::make_shared<Expr>();
  synthetic->kind = ExprKind::kCall;
  synthetic->position = value.position;
  synthetic->children.push_back(callee);
  synthetic->children.push_back(std::make_shared<Expr>(value));
  const std::string inlined = TryInlineCode(klass, *synthetic);
  if (!inlined.empty())
    return StripReturnPrefix(StripTrailingSemicolon(inlined));
  return EmitCall(klass, *synthetic);
}

std::string CxxEmitter::EmitConverted(const ClassInfo& klass, const std::string& expected_type,
                                      const Expr& expression) {
  const std::string expected_base = CxxBaseTypeName(Trim(expected_type));
  if (expression.kind == ExprKind::kNull ||
      (expression.kind == ExprKind::kLiteral && expression.text == "空")) {
    if (expected_base.empty())
      return kCxxNullLiteral;
    if (ReferenceShape()) {
      const std::string mapped = TypeName(klass, expected_type);
      if (mapped.rfind(kCxxSharedPtr, 0) == 0)
        return kCxxNullLiteral;
      ReportUnsupported(
          klass, expression.position,
          "禁用 FreeTiecodec 特性时，值类型的 空 参考形态（return nullptr）不是合法 C++",
          "compat.ftc.null-value");
      return DefaultValue(klass, expected_type);
    }
    return DefaultValue(klass, expected_type);
  }
  if (expression.kind == ExprKind::kLiteral) {
    return EmitLiteral(klass, expression, expected_type);
  }
  if (expression.kind == ExprKind::kArrayLiteral && !expected_type.empty()) {
    return EmitArrayLiteral(klass, expression, expected_type);
  }
  const std::string raw = EmitExpression(klass, expression);
  if (expected_base.empty())
    return raw;
  const std::string source_base = CxxBaseTypeName(StaticTypeOf(klass, expression));
  if (source_base.empty())
    return raw;
  if (IsObjectBase(expected_base) && !IsObjectBase(source_base) &&
      (IsNumericBase(source_base) || IsTextBase(source_base))) {
    return BoxedExpression(klass, source_base, raw);
  }
  return raw;
}

std::string CxxEmitter::EmitLiteral(const ClassInfo& klass, const Expr& expression,
                                    const std::string& expected_type) const {
  const std::string& text = expression.raw.empty() ? expression.text : expression.raw;
  const std::string expected_base = CxxBaseTypeName(Trim(expected_type));
  if (text.empty())
    return text;
  if (text == "真" || text == "假") {
    const std::string value = text == "真" ? "true" : "false";
    if (IsObjectBase(expected_base))
      return BoxedExpression(klass, "逻辑型", value);
    return value;
  }
  if (text == "空")
    return expected_base.empty() ? std::string(kCxxNullLiteral)
                                 : DefaultValue(klass, expected_type);
  if (text.front() == '"' && text.size() >= 2) {
    const std::string literal = EmitStringLiteral(expression.text);
    if (IsObjectBase(expected_base))
      return std::string(kCxxMakeShared) + "<CTieString>(" + literal + ")";
    return literal;
  }
  if (text.front() == '\'') {
    if (IsObjectBase(expected_base))
      return BoxedExpression(klass, "字符", text);
    return text;
  }
  if (LooksNumericLiteral(text)) {
    std::string tie_type = "整数";
    if (text.back() == 'L' || text.back() == 'l')
      tie_type = "长整数";
    else if (text.back() == 'F' || text.back() == 'f')
      tie_type = "单精度小数";
    else if (text.find('.') != std::string::npos)
      tie_type = "小数";
    if (IsObjectBase(expected_base))
      return BoxedExpression(klass, tie_type, text);
    return text;
  }
  return text;
}

std::string CxxEmitter::EmitArrayLiteral(const ClassInfo& klass, const Expr& expression,
                                         const std::string& expected_type) {
  std::string element_tie;
  const std::string expected_base = CxxBaseTypeName(Trim(expected_type));
  if (!expected_type.empty()) {
    const detail::TypeShape shape = detail::ParseTypeShape(Trim(expected_type));
    if (shape.dimensions > 0)
      element_tie = Trim(shape.base);
    else if (!expected_base.empty() && expected_base != "对象")
      element_tie = expected_base;
  }
  if (element_tie.empty() && !expression.children.empty() && expression.children[0] != nullptr)
    element_tie = StaticTypeOf(klass, *expression.children[0]);
  if (element_tie.empty())
    element_tie = "整数";
  const std::string element_cpp = TypeName(klass, element_tie);
  std::string items;
  for (std::size_t i = 0; i < expression.children.size(); ++i) {
    if (i > 0)
      items += ", ";
    if (expression.children[i] != nullptr)
      items += EmitExpressionAs(klass, *expression.children[i], element_tie);
  }
  return std::string(kCxxMakeShared) + "<" + kCxxArrayClass + "<" + element_cpp + ">>(" +
         "std::initializer_list<" + element_cpp + ">{" + items + "})";
}

std::string CxxEmitter::EmitFieldAccess(const ClassInfo& klass, const Expr& expression) {
  if (expression.children.empty() || expression.children[0] == nullptr)
    return expression.text;
  const Expr& receiver = *expression.children[0];
  const std::string receiver_type = StaticTypeOf(klass, receiver);
  const std::string receiver_base = CxxBaseTypeName(receiver_type);
  const ClassInfo* receiver_class =
      receiver_base.empty() ? nullptr : symbols_.Resolve(receiver_base, &klass);

  if (receiver_class != nullptr && receiver.kind == ExprKind::kIdentifier &&
      receiver.text == cxx_names_.ClassName(*receiver_class) &&
      !TryLocalType(receiver.text, nullptr)) {
    const ClassInfo* owner = nullptr;
    const Member* member = FindMemberInChain(symbols_, *receiver_class, expression.text, &owner);
    if (member != nullptr)
      return StaticOwnerRef(*receiver_class) + "::" + cxx_names_.MemberName(*member);
  }

  const std::string receiver_text = EmitExpression(klass, receiver);
  if (receiver_class == nullptr) {
    if (expression.text == "长度")
      return receiver_text + "->GetLength()";
    return receiver_text + "." + expression.text;
  }

  const ClassInfo* owner = nullptr;
  const Member* member = FindMemberInChain(symbols_, *receiver_class, expression.text, &owner);
  if (member != nullptr && member->kind == MemberKind::kPropertyWrite) {
    if (const Member* getter =
            FindPropertyAccessor(*receiver_class, expression.text, MemberKind::kPropertyRead))
      member = getter;
  }
  const std::string access = MemberAccessOperator(klass, receiver_type);
  if (member != nullptr && owner != nullptr) {
    const std::string member_name = cxx_names_.MemberName(*member);
    if (member->kind == MemberKind::kPropertyRead)
      return receiver_text + access + member_name + "()";
    return receiver_text + access + member_name;
  }
  if (const Member* getter =
          FindPropertyAccessor(*receiver_class, expression.text, MemberKind::kPropertyRead)) {
    return receiver_text + access + cxx_names_.MemberName(*getter) + "()";
  }
  if (expression.text == "长度")
    return receiver_text + "->GetLength()";
  return receiver_text + access + expression.text;
}

const CxxMemberInfo* CxxEmitter::ResolveCalledMember(const ClassInfo& klass, const Expr& call,
                                                     const ClassInfo** receiver_class,
                                                     std::string* receiver_type) const {
  if (call.children.empty() || call.children[0] == nullptr)
    return nullptr;
  const Expr& callee = *call.children[0];
  if (callee.kind == ExprKind::kIdentifier) {
    const ClassInfo* owner = nullptr;
    const Member* member = ResolveGlobalMethod(symbols_, klass, callee.text, &owner);
    if (member == nullptr) {
      const CxxMemberInfo* info = LookupMember(klass, callee.text);
      if (info != nullptr) {
        if (receiver_class != nullptr)
          *receiver_class = info->owner;
        if (receiver_type != nullptr)
          receiver_type->clear();
        return info;
      }
      return nullptr;
    }
    static thread_local CxxMemberInfo resolved;
    resolved.member = member;
    resolved.owner = owner;
    resolved.type_name = member->type_name;
    resolved.is_field = false;
    const CxxModifiers* modifiers = effects_.Get<CxxModifiers>(static_cast<const void*>(member));
    resolved.is_static_member = modifiers != nullptr && modifiers->is_static;
    if (receiver_class != nullptr)
      *receiver_class = owner;
    if (receiver_type != nullptr)
      receiver_type->clear();
    return &resolved;
  }
  if (callee.kind == ExprKind::kFieldAccess && !callee.children.empty() &&
      callee.children[0] != nullptr) {
    const Expr& receiver = *callee.children[0];
    const std::string type = StaticTypeOf(klass, receiver);
    std::string base = CxxBaseTypeName(type);
    const ClassInfo* target = base.empty() ? nullptr : symbols_.Resolve(base, &klass);
    if (target == nullptr && receiver.kind == ExprKind::kIdentifier) {
      target = symbols_.Resolve(receiver.text, &klass);
      if (target != nullptr && receiver_class != nullptr)
        *receiver_class = target;
      const CxxMemberInfo* info = LookupMember(*target, callee.text);
      if (receiver_type != nullptr)
        receiver_type->clear();
      return info;
    }
    if (target == nullptr)
      return nullptr;
    if (receiver_class != nullptr)
      *receiver_class = target;
    if (receiver_type != nullptr)
      *receiver_type = type;
    return LookupMember(*target, callee.text);
  }
  return nullptr;
}

const CxxMemberInfo* CxxEmitter::ResolveCalledMemberInner(Expr& call,
                                                          const ClassInfo** receiver_class,
                                                          std::string* receiver_type) const {
  const ClassInfo* klass = context_.klass;
  if (klass == nullptr)
    return nullptr;
  return ResolveCalledMember(*klass, call, receiver_class, receiver_type);
}

std::string CxxEmitter::EmitCall(const ClassInfo& klass, const Expr& expression) {
  if (expression.children.empty() || expression.children[0] == nullptr)
    return "/* 调用缺失 */";
  const Expr& callee = *expression.children[0];
  const ClassInfo* owner = nullptr;
  std::string receiver_type;
  const CxxMemberInfo* info = ResolveCalledMember(klass, expression, &owner, &receiver_type);

  if (info == nullptr || info->member == nullptr || owner == nullptr) {
    if (callee.kind == ExprKind::kFieldAccess && !callee.children.empty() &&
        callee.children[0] != nullptr) {
      const std::string receiver_text = EmitExpression(klass, *callee.children[0]);
      const std::string access = MemberAccessOperator(klass, receiver_type);
      std::string args;
      for (std::size_t i = 1; i < expression.children.size(); ++i) {
        if (i > 1)
          args += ", ";
        if (expression.children[i] != nullptr)
          args += EmitExpression(klass, *expression.children[i]);
      }
      return receiver_text + access + callee.text + "(" + args + ")";
    }
    ReportUnsupported(klass, expression.position, "无法解析调用的成员 \"" + callee.text + "\"",
                      "codegen.unresolved-call");
    return callee.text + "()";
  }

  const Member& member = *info->member;
  std::string args;
  for (std::size_t i = 0; i < member.parameters.size(); ++i) {
    if (i > 0)
      args += ", ";
    const ExprPtr& provided =
        i + 1 < expression.children.size() ? expression.children[i + 1] : nullptr;
    if (provided != nullptr) {
      args += EmitConverted(klass, member.parameters[i].type_name, *provided);
      continue;
    }
    if (member.parameters[i].default_value != nullptr) {
      const ClassInfo* saved = context_.klass;
      args += EmitConverted(*owner, member.parameters[i].type_name,
                            *member.parameters[i].default_value);
      (void)saved;
      continue;
    }
    args += DefaultValue(klass, member.parameters[i].type_name);
  }

  const bool receiver_is_class_name = callee.kind == ExprKind::kIdentifier &&
                                      symbols_.Resolve(callee.text, &klass) != nullptr &&
                                      !TryLocalType(callee.text, nullptr);
  if (info->is_static_member || receiver_is_class_name) {
    return StaticOwnerRef(*owner) + "::" + cxx_names_.MemberName(member) + "(" + args + ")";
  }

  const bool external_generic_owner = owner->HasExternalType(kCxxTypeAliasKey) &&
                                      owner->declaration != nullptr &&
                                      !owner->declaration->generic_parameters.empty();
  if (external_generic_owner && ReferenceShape() && !receiver_type.empty()) {
    std::string spec_args;
    const std::size_t open = receiver_type.find('<');
    const std::size_t close = receiver_type.rfind('>');
    if (open != std::string::npos && close != std::string::npos && close > open) {
      for (const std::string& argument :
           CxxSplitTypeArguments(receiver_type.substr(open + 1, close - open - 1))) {
        if (!spec_args.empty())
          spec_args += ",";
        spec_args += TypeName(klass, argument);
      }
    }
    const std::string specialization =
        cxx_names_.ClassName(*owner) + (spec_args.empty() ? "" : "<" + spec_args + ">");
    const std::string receiver_text = !callee.children.empty() && callee.children[0] != nullptr
                                          ? EmitExpression(klass, *callee.children[0])
                                          : std::string("this");
    std::string call_args = receiver_text;
    if (!args.empty())
      call_args += ", " + args;
    return specialization + "::" + cxx_names_.MemberName(member) + "(" + call_args + ")";
  }

  if (callee.kind == ExprKind::kIdentifier) {
    if (owner == &klass)
      return "this->" + cxx_names_.MemberName(member) + "(" + args + ")";
    return StaticOwnerRef(*owner) + "::" + cxx_names_.MemberName(member) + "(" + args + ")";
  }

  if (!callee.children.empty() && callee.children[0] != nullptr &&
      callee.children[0]->kind == ExprKind::kSuper) {
    const std::string base = cxx_names_.BaseName(klass);
    return "this->" + base + "::" + cxx_names_.MemberName(member) + "(" + args + ")";
  }

  const std::string receiver_text = !callee.children.empty() && callee.children[0] != nullptr
                                        ? EmitExpression(klass, *callee.children[0])
                                        : std::string("this");
  const std::string access = MemberAccessOperator(klass, receiver_type);
  return receiver_text + access + cxx_names_.MemberName(member) + "(" + args + ")";
}

std::string CxxEmitter::EmitArrayAccess(const ClassInfo& klass, const Expr& expression) {
  if (expression.children.size() < 2 || expression.children[0] == nullptr ||
      expression.children[1] == nullptr) {
    return "/* 数组访问缺失 */";
  }
  ExprPtr receiver = expression.children[0];
  ExprPtr index = expression.children[1];
  const std::string receiver_type = StaticTypeOf(klass, *receiver);
  const std::string base = CxxBaseTypeName(receiver_type);
  const ClassInfo* receiver_class = symbols_.Resolve(base, &klass);

  if (receiver_class != nullptr) {
    const ClassInfo* owner = nullptr;
    const Member* getter = FindMemberInChain(symbols_, *receiver_class, kOperatorIndex, &owner);
    if (getter != nullptr && IsInlineMember(*getter)) {
      Expr synthetic;
      synthetic.kind = ExprKind::kCall;
      synthetic.position = expression.position;
      ExprPtr callee = std::make_shared<Expr>();
      callee->kind = ExprKind::kFieldAccess;
      callee->text = kOperatorIndex;
      callee->position = expression.position;
      callee->children.push_back(receiver);
      synthetic.children.push_back(callee);
      synthetic.children.push_back(index);
      const std::string inlined = TryInlineCode(klass, synthetic);
      if (!inlined.empty())
        return StripReturnPrefix(StripTrailingSemicolon(inlined));
    }
  }
  const std::string receiver_text = EmitExpression(klass, *receiver);
  const std::string index_text = EmitExpressionAs(klass, *index, "整数");
  return receiver_text + "->operator[](" + index_text + ")";
}

std::string CxxEmitter::EmitAssign(const ClassInfo& klass, const Expr& expression) {
  if (expression.children.size() < 2 || expression.children[0] == nullptr ||
      expression.children[1] == nullptr) {
    return "/* 赋值缺失 */";
  }
  const Expr& target = *expression.children[0];
  const Expr& value = *expression.children[1];

  if (target.kind == ExprKind::kNew) {
    const ClassInfo* target_class = symbols_.Resolve(CxxBaseTypeName(target.text), &klass);
    if (target_class != nullptr) {
      const ClassInfo* owner = nullptr;
      const Member* assign_op = FindMemberInChain(symbols_, *target_class, kOperatorAssign, &owner);
      if (assign_op != nullptr) {
        ExprPtr receiver = std::make_shared<Expr>();
        receiver->kind = ExprKind::kIdentifier;
        receiver->text = CxxBaseTypeName(target.text);
        receiver->raw = receiver->text;
        receiver->position = target.position;
        ExprPtr callee = std::make_shared<Expr>();
        callee->kind = ExprKind::kFieldAccess;
        callee->text = kOperatorAssign;
        callee->position = target.position;
        callee->children.push_back(receiver);
        ExprPtr synthetic = std::make_shared<Expr>();
        synthetic->kind = ExprKind::kCall;
        synthetic->position = expression.position;
        synthetic->children.push_back(callee);
        synthetic->children.push_back(std::make_shared<Expr>(value));
        const std::string inlined = TryInlineCode(klass, *synthetic);
        if (!inlined.empty())
          return StripReturnPrefix(StripTrailingSemicolon(inlined));
        return EmitCall(klass, *synthetic);
      }
    }
    return EmitExpression(klass, target) + " = " + EmitConverted(klass, target.text, value);
  }

  if (target.kind == ExprKind::kArrayAccess && target.children.size() >= 2 &&
      target.children[0] != nullptr && target.children[1] != nullptr) {
    const std::string receiver_type = StaticTypeOf(klass, *target.children[0]);
    const std::string base = CxxBaseTypeName(receiver_type);
    const ClassInfo* receiver_class = symbols_.Resolve(base, &klass);
    if (receiver_class != nullptr) {
      const ClassInfo* owner = nullptr;
      const Member* setter =
          FindMemberInChain(symbols_, *receiver_class, kOperatorIndexAssign, &owner);
      if (setter != nullptr) {
        Expr synthetic;
        synthetic.kind = ExprKind::kCall;
        synthetic.position = expression.position;
        ExprPtr callee = std::make_shared<Expr>();
        callee->kind = ExprKind::kFieldAccess;
        callee->text = kOperatorIndexAssign;
        callee->position = expression.position;
        callee->children.push_back(target.children[0]);
        synthetic.children.push_back(callee);
        synthetic.children.push_back(target.children[1]);
        synthetic.children.push_back(expression.children[1]);
        if (IsInlineMember(*setter)) {
          const std::string inlined = TryInlineCode(klass, synthetic);
          if (!inlined.empty())
            return StripTrailingSemicolon(inlined);
        } else {
          return EmitCall(klass, synthetic);
        }
      }
    }
    const std::string element = ElementTypeOf(klass, receiver_type);
    const std::string receiver_text = EmitExpression(klass, *target.children[0]);
    const std::string index_text = EmitExpressionAs(klass, *target.children[1], "整数");
    return receiver_text + "->operator[](" + index_text +
           ") = " + EmitConverted(klass, element, value);
  }

  if (target.kind == ExprKind::kFieldAccess && !target.children.empty() &&
      target.children[0] != nullptr) {
    const std::string receiver_type = StaticTypeOf(klass, *target.children[0]);
    const std::string base = CxxBaseTypeName(receiver_type);
    const ClassInfo* receiver_class = symbols_.Resolve(base, &klass);
    if (receiver_class != nullptr) {
      const Member* setter =
          FindPropertyAccessor(*receiver_class, target.text, MemberKind::kPropertyWrite);
      if (setter != nullptr) {
        Expr synthetic;
        synthetic.kind = ExprKind::kCall;
        synthetic.position = expression.position;
        ExprPtr callee = std::make_shared<Expr>();
        callee->kind = ExprKind::kFieldAccess;
        callee->text = target.text;
        callee->position = expression.position;
        callee->children.push_back(target.children[0]);
        synthetic.children.push_back(callee);
        synthetic.children.push_back(expression.children[1]);
        if (IsInlineMember(*setter)) {
          const std::string inlined = TryInlineCode(klass, synthetic);
          if (!inlined.empty())
            return StripTrailingSemicolon(inlined);
        }
        return EmitCall(klass, synthetic);
      }
    }
    const std::string target_type = StaticTypeOf(klass, target);
    return EmitExpression(klass, target) + " = " + EmitConverted(klass, target_type, value);
  }

  const std::string target_type = StaticTypeOf(klass, target);
  return EmitExpression(klass, target) + " = " + EmitConverted(klass, target_type, value);
}

std::string CxxEmitter::EmitBinary(const ClassInfo& klass, const Expr& expression) {
  if (expression.children.size() < 2 || expression.children[0] == nullptr ||
      expression.children[1] == nullptr) {
    return "/* 二元表达式缺失 */";
  }
  const Expr& left = *expression.children[0];
  const Expr& right = *expression.children[1];
  const std::string op = expression.text;

  std::string cpp_op = op;
  if (op == "且")
    cpp_op = "&&";
  else if (op == "或")
    cpp_op = "||";

  if (op == "+" && left.kind == ExprKind::kLiteral && right.kind == ExprKind::kLiteral &&
      !left.raw.empty() && left.raw.front() == '"' && !right.raw.empty() &&
      right.raw.front() == '"') {
    return EmitStringLiteral(left.text + right.text);
  }

  const std::string left_type = StaticTypeOf(klass, left);
  const std::string left_base = CxxBaseTypeName(left_type);
  if (!left_base.empty()) {
    const ClassInfo* left_class = symbols_.Resolve(left_base, &klass);
    if (left_class != nullptr) {
      const ClassInfo* owner = nullptr;
      const Member* overload = FindMemberInChain(symbols_, *left_class, op, &owner);
      if (overload != nullptr) {
        ExprPtr left_copy = std::make_shared<Expr>(left);
        ExprPtr right_copy = std::make_shared<Expr>(right);
        ExprPtr callee = std::make_shared<Expr>();
        callee->kind = ExprKind::kFieldAccess;
        callee->text = op;
        callee->position = expression.position;
        callee->children.push_back(left_copy);
        ExprPtr synthetic = std::make_shared<Expr>();
        synthetic->kind = ExprKind::kCall;
        synthetic->position = expression.position;
        synthetic->children.push_back(callee);
        synthetic->children.push_back(right_copy);
        std::string result;
        if (IsInlineMember(*overload))
          result = TryInlineCode(klass, *synthetic);
        if (result.empty())
          result = EmitCall(klass, *synthetic);
        if (!result.empty())
          return StripReturnPrefix(StripTrailingSemicolon(result));
      }
    }
  }

  const std::string left_text = EmitExpression(klass, left);
  std::string right_text;
  if (op == "+" && IsTextBase(left_base))
    right_text = EmitConverted(klass, "文本", right);
  else
    right_text = EmitExpression(klass, right);
  const bool spaced = cpp_op == "+" || cpp_op == "-" || cpp_op == "<" || cpp_op == ">" ||
                      cpp_op == "<=" || cpp_op == ">=" || cpp_op == "==" || cpp_op == "!=" ||
                      cpp_op == "&&" || cpp_op == "||";
  return left_text + (spaced ? " " : "") + cpp_op + (spaced ? " " : "") + right_text;
}

std::string CxxEmitter::EmitNew(const ClassInfo& klass, const Expr& expression) {
  const std::string type_name = Trim(expression.text);
  const detail::TypeShape shape = detail::ParseTypeShape(type_name);
  const std::string base = Trim(shape.base);
  if (shape.dimensions > 0) {
    const std::string element_cpp = TypeName(klass, base);
    std::string capacity = "0";
    if (!shape.capacities.empty() && !Trim(shape.capacities[0]).empty())
      capacity = Trim(shape.capacities[0]);
    return std::string(kCxxMakeShared) + "<" + kCxxArrayClass + "<" + element_cpp + ">>(" +
           capacity + ", " + DefaultValue(klass, base) + ")";
  }
  const ClassInfo* target = base.empty() ? nullptr : symbols_.Resolve(base, &klass);
  std::string inner;
  if (target == nullptr) {
    inner = TypeName(klass, type_name);
    if (inner.rfind(kCxxSharedPtr, 0) == 0)
      inner = inner.substr(std::string(kCxxSharedPtr).size() + 1,
                           inner.size() - std::string(kCxxSharedPtr).size() - 2);
  } else if (IsExternalClass(*target)) {
    const std::string external = target->ExternalType(kCxxTypeAliasKey);
    if (target->declaration != nullptr && !target->declaration->generic_parameters.empty()) {
      std::string args;
      const std::size_t open = type_name.find('<');
      const std::size_t close = type_name.rfind('>');
      if (open != std::string::npos && close != std::string::npos && close > open) {
        for (const std::string& argument :
             CxxSplitTypeArguments(type_name.substr(open + 1, close - open - 1))) {
          if (!args.empty())
            args += ", ";
          args += TypeName(klass, argument);
        }
      }
      inner = args.empty() ? external : external + "<" + args + ">";
    } else {
      inner = external;
    }
  } else if (target->declaration != nullptr && !target->declaration->generic_parameters.empty()) {
    std::string args;
    const std::size_t open = type_name.find('<');
    const std::size_t close = type_name.rfind('>');
    if (open != std::string::npos && close != std::string::npos && close > open) {
      for (const std::string& argument :
           CxxSplitTypeArguments(type_name.substr(open + 1, close - open - 1))) {
        if (!args.empty())
          args += ", ";
        args += TypeName(klass, argument);
      }
    }
    inner = cxx_names_.ClassName(*target);
    if (!args.empty())
      inner += "<" + args + ">";
  } else {
    inner = ClassValueName(*target);
  }
  return std::string(kCxxMakeShared) + "<" + inner + ">()";
}

std::string CxxEmitter::TryInlineCode(const ClassInfo& klass, const Expr& call) {
  if (call.kind != ExprKind::kCall || call.children.empty() || call.children[0] == nullptr)
    return {};
  const ClassInfo* owner = nullptr;
  std::string receiver_type;
  const CxxMemberInfo* info = ResolveCalledMember(klass, call, &owner, &receiver_type);
  if (info == nullptr || info->member == nullptr)
    return {};
  const Member& member = *info->member;
  std::string body;
  if (IsInlineMember(member)) {
    if (!member.code_text.empty()) {
      body = member.code_text;
    } else if (member.body.size() == 1 && member.body[0] != nullptr &&
               member.body[0]->kind == StmtKind::kCode) {
      body = member.body[0]->code_text;
    } else {
      for (const StmtPtr& statement : member.body) {
        if (statement != nullptr && statement->kind == StmtKind::kCode)
          return {};
      }
      return {};
    }
  } else if (owner != nullptr && owner->HasExternalType(kCxxTypeAliasKey)) {
    if (member.body.size() == 1 && member.body[0] != nullptr &&
        member.body[0]->kind == StmtKind::kCode) {
      body = member.body[0]->code_text;
    }
  }
  if (body.empty())
    return {};

  const Expr& callee = *call.children[0];
  std::string receiver_text;
  bool receiver_value = false;
  if (callee.kind == ExprKind::kFieldAccess && !callee.children.empty() &&
      callee.children[0] != nullptr) {
    if (info->is_static_member) {
      receiver_text.clear();
    } else {
      receiver_text = EmitExpression(klass, *callee.children[0]);
      receiver_value = ReceiverIsValue(klass, receiver_type);
    }
  }

  auto saved_parameters = current_parameters_;
  auto saved_local_types = local_types_;
  const std::string saved_receiver = inline_receiver_;
  const bool saved_receiver_value = inline_receiver_value_;
  const bool saved_substitution = inline_substitution_;
  const int saved_line = current_code_line_;

  current_parameters_.clear();
  if (owner != nullptr && owner->declaration != nullptr &&
      !owner->declaration->generic_parameters.empty() && !receiver_type.empty()) {
    const std::size_t open = receiver_type.find('<');
    const std::size_t close = receiver_type.rfind('>');
    if (open != std::string::npos && close != std::string::npos && close > open) {
      const std::vector<std::string> arguments =
          CxxSplitTypeArguments(receiver_type.substr(open + 1, close - open - 1));
      for (std::size_t i = 0;
           i < owner->declaration->generic_parameters.size() && i < arguments.size(); ++i) {
        current_parameters_[owner->declaration->generic_parameters[i]] =
            TypeName(klass, arguments[i]);
      }
    }
  }
  for (std::size_t i = 0; i < member.parameters.size(); ++i) {
    const Parameter& parameter = member.parameters[i];
    const ExprPtr& provided = i + 1 < call.children.size() ? call.children[i + 1] : nullptr;
    if (provided != nullptr) {
      current_parameters_[parameter.name] = EmitConverted(klass, parameter.type_name, *provided);
    } else if (parameter.default_value != nullptr) {
      current_parameters_[parameter.name] =
          EmitConverted(klass, parameter.type_name, *parameter.default_value);
    } else {
      current_parameters_[parameter.name] = DefaultValue(klass, parameter.type_name);
    }
  }
  inline_receiver_ = receiver_text;
  inline_receiver_value_ = receiver_value;
  inline_substitution_ = true;
  current_code_line_ = member.code_line;

  std::string result = SubstituteCodeInner(klass, body, false);
  const std::vector<std::string> lines =
      detail::NormalizeCodeBlock(result, 0, detail::CodeIndentOf(member));
  std::string normalized;
  for (std::size_t i = 0; i < lines.size(); ++i) {
    if (i > 0)
      normalized += "\n";
    normalized += TrimTrailing(lines[i]);
  }

  current_parameters_ = saved_parameters;
  local_types_ = saved_local_types;
  inline_receiver_ = saved_receiver;
  inline_receiver_value_ = saved_receiver_value;
  inline_substitution_ = saved_substitution;
  current_code_line_ = saved_line;
  return normalized;
}

std::string CxxEmitter::SubstituteCode(const ClassInfo& klass, const std::string& code) {
  return SubstituteCodeInner(klass, code, false);
}

std::string CxxEmitter::SubstituteCodeInner(const ClassInfo& klass, const std::string& code,
                                            bool strip_return) {
  std::vector<std::pair<std::string, std::string>> ordered(current_parameters_.begin(),
                                                           current_parameters_.end());
  std::sort(
      ordered.begin(), ordered.end(),
      [](const std::pair<std::string, std::string>& a,
         const std::pair<std::string, std::string>& b) { return a.first.size() > b.first.size(); });

  std::string out;
  std::size_t i = 0;
  const ClassInfo& current = klass;
  while (i < code.size()) {
    const char c = code[i];
    if (c == '"' || c == '\'') {
      out += c;
      ++i;
      while (i < code.size()) {
        out += code[i];
        if (code[i] == '\\' && i + 1 < code.size()) {
          ++i;
          out += code[i];
        } else if (code[i] == c) {
          ++i;
          break;
        }
        ++i;
      }
      continue;
    }
    if (c != '#') {
      out += c;
      ++i;
      continue;
    }
    ++i;
    if (i >= code.size()) {
      out += '#';
      break;
    }
    std::string name;
    while (i < code.size() && detail::IsIdentifierByte(code[i])) {
      name += code[i];
      ++i;
    }
    if (name.empty()) {
      out += '#';
      continue;
    }
    if ((name == "cls" || name == "ncls" || name == "mem" || name == "expr" || name == "get" ||
         name == "set" || name == "sys") &&
        i < code.size() && code[i] == '<') {
      std::string content;
      if (!detail::ReadAngleContent(code, &i, &content)) {
        out += "#" + name;
        continue;
      }
      if (name == "cls" || name == "ncls") {
        const auto bound = current_parameters_.find(content);
        if (bound != current_parameters_.end())
          out += bound->second;
        else
          out += TypeName(current, content);
      } else if (name == "mem") {
        const std::size_t dot = content.rfind('.');
        if (dot != std::string::npos) {
          const std::string class_name = content.substr(0, dot);
          const std::string member_name = content.substr(dot + 1);
          const ClassInfo* target = symbols_.Resolve(class_name, &current);
          if (target != nullptr) {
            const ClassInfo* owner = nullptr;
            const Member* member = FindMemberInChain(symbols_, *target, member_name, &owner);
            if (member != nullptr) {
              out += StaticOwnerRef(*target) + "::" + cxx_names_.MemberName(*member);
            } else {
              out += content;
            }
          } else {
            out += content;
          }
        } else {
          out += content;
        }
      } else if (name == "expr") {
        const ClassInfo* owner = nullptr;
        std::vector<std::string> tokens;
        std::string token;
        for (char inner : content) {
          if (detail::IsIdentifierByte(inner)) {
            token += inner;
          } else {
            if (!token.empty())
              tokens.push_back(token);
            token.clear();
            tokens.push_back(std::string(1, inner));
          }
        }
        if (!token.empty())
          tokens.push_back(token);
        for (const std::string& piece : tokens) {
          if (!piece.empty() && detail::IsIdentifierByte(piece[0])) {
            const auto found = current_parameters_.find(piece);
            if (found != current_parameters_.end()) {
              out += found->second;
              (void)owner;
            } else {
              out += TranslateName(current, piece);
            }
          } else {
            out += piece;
          }
        }
      } else if (name == "get" || name == "set") {
        const MemberKind want =
            name == "get" ? MemberKind::kPropertyRead : MemberKind::kPropertyWrite;
        const ClassInfo* cursor = &current;
        bool matched = false;
        for (std::size_t depth = 0; cursor != nullptr && cursor->declaration != nullptr; ++depth) {
          const auto found = cursor->member_by_name.find(content);
          if (found != cursor->member_by_name.end() && found->second->kind == want) {
            out += cxx_names_.MemberName(*found->second);
            matched = true;
            break;
          }
          if (cursor->declaration->base_name.empty())
            break;
          cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
          if (depth > symbols_.class_count())
            break;
        }
        if (!matched)
          out += content;
      } else {
        if (content == "line") {
          out += std::to_string(
              current_code_line_ > 0
                  ? current_code_line_
                  : (klass.declaration != nullptr ? klass.declaration->position.line + 1 : 0));
        } else if (content == "source") {
          out += klass.declaration != nullptr ? klass.declaration->file : std::string();
        } else if (content == "main_pkg") {
          out += config_.namespace_name;
        } else if (content == "time") {
          const auto now = std::chrono::system_clock::now().time_since_epoch();
          const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
          out += std::to_string(millis) + "LL";
        } else {
          out += content;
        }
      }
      continue;
    }
    if (name == "this") {
      if (inline_substitution_ && !inline_receiver_.empty()) {
        out += inline_receiver_;
        if (inline_receiver_value_) {
          std::size_t probe = i;
          while (probe < code.size() && (code[probe] == ' ' || code[probe] == '\t'))
            ++probe;
          if (probe + 1 < code.size() && code[probe] == '-' && code[probe + 1] == '>') {
            out += ". ";
            i = probe + 2;
            continue;
          }
        }
      } else {
        out += "this";
      }
      continue;
    }
    const auto found = std::find_if(
        ordered.begin(), ordered.end(),
        [&name](const std::pair<std::string, std::string>& entry) { return entry.first == name; });
    if (found != ordered.end()) {
      out += found->second;
      continue;
    }
    const CxxMemberInfo* info = LookupMember(current, name);
    if (info != nullptr && info->is_static_member && info->owner != nullptr) {
      out += StaticOwnerRef(*info->owner) + "::" + cxx_names_.MemberName(*info->member);
      continue;
    }
    if (info != nullptr) {
      out += cxx_names_.MemberName(*info->member);
      continue;
    }
    out += name;
  }
  std::string result = out;
  if (strip_return) {
    const std::string prefix = "return ";
    const std::size_t first = result.find_first_not_of(" \t\n");
    if (first != std::string::npos && result.compare(first, prefix.size(), prefix) == 0)
      result = result.substr(first + prefix.size());
  }
  return result;
}

}
