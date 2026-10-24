#ifndef TIECODEC_TOKEN_H
#define TIECODEC_TOKEN_H

#include <cstddef>
#include <string>

#include "position.h"

namespace tiecodec {

enum class TokenKind {
  kEnd,
  kIdentifier,
  kInteger,
  kLong,
  kFloat,
  kDouble,
  kChar,
  kString,
  kLongString,

  kCodeBlock,

  kSymbol,
};

struct Token {
  TokenKind kind = TokenKind::kEnd;

  std::string text;

  std::string raw;
  SourcePosition position;

  bool begins_line = false;

  int indent_level = 0;
};

const char* TokenKindName(TokenKind kind);

}

#endif
