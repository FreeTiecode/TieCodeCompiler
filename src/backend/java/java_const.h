#ifndef TIECODEC_BACKEND_JAVA_CONST_H
#define TIECODEC_BACKEND_JAVA_CONST_H

#include <string>

#include "ast.h"

namespace tiecodec {

struct ConstantValue {
  enum class Kind { kInteger, kDouble, kBoolean };

  Kind kind = Kind::kInteger;
  long long integer = 0;
  double real = 0.0;
  bool boolean = false;
};

/// Folds a constant expression; returns false for non-constant expressions.
bool FoldConstant(const Expr& expression, ConstantValue* value);

/// Renders a folded value the same way the reference compiler prints literals.
std::string FormatConstant(const ConstantValue& value);

/// Left-most source position of an expression tree (used by diagnostics).
SourcePosition ExpressionStart(const Expr& expression);

}  // namespace tiecodec

#endif
