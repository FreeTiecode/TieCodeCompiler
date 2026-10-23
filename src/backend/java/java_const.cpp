#include "java_const.h"

#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>

namespace tiecodec {
namespace {

bool IsNumericText(const std::string& text, bool* is_double) {
  if (text.empty())
    return false;
  std::size_t index = 0;
  if (text[index] == '-' || text[index] == '+')
    ++index;
  bool digits = false;
  bool dot = false;
  bool exponent = false;
  for (; index < text.size(); ++index) {
    const char c = text[index];
    if (c >= '0' && c <= '9') {
      digits = true;
      continue;
    }
    if (c == '.' && !dot) {
      dot = true;
      continue;
    }
    if ((c == 'e' || c == 'E') && digits && !exponent) {
      exponent = true;
      if (index + 1 < text.size() && (text[index + 1] == '+' || text[index + 1] == '-')) {
        ++index;
      }
      continue;
    }
    if ((c == 'f' || c == 'F' || c == 'd' || c == 'D' || c == 'l' || c == 'L') &&
        index + 1 == text.size()) {
      break;
    }
    return false;
  }
  *is_double = dot || exponent;
  return digits;
}

bool AsNumeric(const ConstantValue& value, double* real, long long* integer, bool* is_double) {
  if (value.kind == ConstantValue::Kind::kBoolean)
    return false;
  if (value.kind == ConstantValue::Kind::kDouble) {
    *real = value.real;
    *is_double = true;
    return true;
  }
  *integer = value.integer;
  *is_double = false;
  return true;
}

}  // namespace

bool FoldConstant(const Expr& expression, ConstantValue* value) {
  switch (expression.kind) {
    case ExprKind::kLiteral: {
      const std::string& text = expression.text;
      if (text == "真") {
        value->kind = ConstantValue::Kind::kBoolean;
        value->boolean = true;
        return true;
      }
      if (text == "假") {
        value->kind = ConstantValue::Kind::kBoolean;
        value->boolean = false;
        return true;
      }
      if (!expression.raw.empty() && (expression.raw[0] == '"' || expression.raw[0] == '\'')) {
        return false;
      }
      bool is_double = false;
      if (!IsNumericText(text, &is_double))
        return false;
      char* end = nullptr;
      if (is_double) {
        value->kind = ConstantValue::Kind::kDouble;
        value->real = std::strtod(text.c_str(), &end);
        return end != nullptr && *end == '\0';
      }
      value->kind = ConstantValue::Kind::kInteger;
      value->integer = std::strtoll(text.c_str(), &end, 10);
      return end != nullptr && *end == '\0';
    }
    case ExprKind::kParen: {
      if (expression.children.size() != 1)
        return false;
      return FoldConstant(*expression.children[0], value);
    }
    case ExprKind::kBinary: {
      if (expression.children.size() != 2)
        return false;
      ConstantValue left;
      ConstantValue right;
      if (!FoldConstant(*expression.children[0], &left))
        return false;
      if (!FoldConstant(*expression.children[1], &right))
        return false;
      const std::string& op = expression.text;
      if (op == "且" || op == "&&" || op == "或" || op == "||") {
        if (left.kind != ConstantValue::Kind::kBoolean ||
            right.kind != ConstantValue::Kind::kBoolean) {
          return false;
        }
        value->kind = ConstantValue::Kind::kBoolean;
        if (op == "且" || op == "&&") {
          value->boolean = left.boolean && right.boolean;
        } else {
          value->boolean = left.boolean || right.boolean;
        }
        return true;
      }
      if (op == "==" || op == "!=" || op == "<" || op == "<=" || op == ">" || op == ">=") {
        double left_real = 0.0;
        double right_real = 0.0;
        long long left_int = 0;
        long long right_int = 0;
        bool left_double = false;
        bool right_double = false;
        bool comparable = false;
        if (left.kind == ConstantValue::Kind::kBoolean &&
            right.kind == ConstantValue::Kind::kBoolean && (op == "==" || op == "!=")) {
          value->kind = ConstantValue::Kind::kBoolean;
          value->boolean =
              op == "==" ? left.boolean == right.boolean : left.boolean != right.boolean;
          return true;
        }
        if (AsNumeric(left, &left_real, &left_int, &left_double) &&
            AsNumeric(right, &right_real, &right_int, &right_double)) {
          comparable = true;
        }
        if (!comparable)
          return false;
        bool result = false;
        if (left_double || right_double) {
          const double a = left_double ? left_real : static_cast<double>(left_int);
          const double b = right_double ? right_real : static_cast<double>(right_int);
          if (op == "==")
            result = a == b;
          if (op == "!=")
            result = a != b;
          if (op == "<")
            result = a < b;
          if (op == "<=")
            result = a <= b;
          if (op == ">")
            result = a > b;
          if (op == ">=")
            result = a >= b;
        } else {
          const long long a = left_int;
          const long long b = right_int;
          if (op == "==")
            result = a == b;
          if (op == "!=")
            result = a != b;
          if (op == "<")
            result = a < b;
          if (op == "<=")
            result = a <= b;
          if (op == ">")
            result = a > b;
          if (op == ">=")
            result = a >= b;
        }
        value->kind = ConstantValue::Kind::kBoolean;
        value->boolean = result;
        return true;
      }
      if (op != "+" && op != "-" && op != "*" && op != "/" && op != "%")
        return false;
      double left_real = 0.0;
      double right_real = 0.0;
      long long left_int = 0;
      long long right_int = 0;
      bool left_double = false;
      bool right_double = false;
      if (!AsNumeric(left, &left_real, &left_int, &left_double))
        return false;
      if (!AsNumeric(right, &right_real, &right_int, &right_double))
        return false;
      if (left_double || right_double) {
        const double a = left_double ? left_real : static_cast<double>(left_int);
        const double b = right_double ? right_real : static_cast<double>(right_int);
        value->kind = ConstantValue::Kind::kDouble;
        if (op == "+")
          value->real = a + b;
        if (op == "-")
          value->real = a - b;
        if (op == "*")
          value->real = a * b;
        if (op == "/") {
          if (b == 0.0)
            return false;
          value->real = a / b;
        }
        if (op == "%")
          return false;
        return true;
      }
      value->kind = ConstantValue::Kind::kInteger;
      if (op == "+")
        value->integer = left_int + right_int;
      if (op == "-")
        value->integer = left_int - right_int;
      if (op == "*")
        value->integer = left_int * right_int;
      if (op == "/") {
        if (right_int == 0)
          return false;
        value->integer = left_int / right_int;
      }
      if (op == "%") {
        if (right_int == 0)
          return false;
        value->integer = left_int % right_int;
      }
      return true;
    }
    default:
      return false;
  }
}

SourcePosition ExpressionStart(const Expr& expression) {
  const Expr* current = &expression;
  while (!current->children.empty() && current->children[0] != nullptr) {
    switch (current->kind) {
      case ExprKind::kBinary:
      case ExprKind::kParen:
      case ExprKind::kUnary:
      case ExprKind::kArrayAccess:
      case ExprKind::kCall:
      case ExprKind::kFieldAccess:
        current = current->children[0].get();
        continue;
      default:
        break;
    }
    break;
  }
  return current->position;
}

std::string FormatConstant(const ConstantValue& value) {
  if (value.kind == ConstantValue::Kind::kBoolean)
    return value.boolean ? "true" : "false";
  if (value.kind == ConstantValue::Kind::kInteger)
    return std::to_string(value.integer);
  if (std::isfinite(value.real) && std::floor(value.real) == value.real &&
      std::fabs(value.real) < 1e15) {
    return std::to_string(static_cast<long long>(value.real));
  }
  char buffer[64];
  std::snprintf(buffer, sizeof(buffer), "%.17g", value.real);
  std::string text(buffer);
  if (text.find('.') == std::string::npos && text.find('e') == std::string::npos) {
    text += ".0";
  }
  return text + "D";
}

}  // namespace tiecodec
