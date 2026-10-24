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

bool FoldConstant(const Expr& expression, ConstantValue* value);

std::string FormatConstant(const ConstantValue& value);

SourcePosition ExpressionStart(const Expr& expression);

}

#endif
