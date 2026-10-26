#ifndef TIECODEC_LEXER_H
#define TIECODEC_LEXER_H

#include <cstddef>
#include <deque>
#include <string>
#include <string_view>

#include "diagnostics.h"
#include "token.h"

namespace tiecodec {

class Lexer {
public:
  Lexer(std::string source, std::string file_name, Diagnostics* diagnostics);

  const Token& Peek(std::size_t count = 0) {
    while (buffer_.size() <= count) {
      buffer_.push_back(Lex());
    }
    return buffer_[count];
  }

  Token Next();

  std::string ReadRestOfLine();

  void SkipRestOfLine();

  const std::string& file_name() const {
    return file_name_;
  }

private:
  Token Lex();
  Token MakeToken(TokenKind kind, std::size_t begin_offset, std::size_t begin_line,
                  std::size_t begin_column) const;

  int IndentLevelAt(std::size_t line_start, std::size_t offset) const;

  void SkipWhitespaceAndComments();
  void ReportError(const std::string& message);

  char Current() const;
  char PeekChar(std::size_t offset = 1) const;
  bool AtEnd() const;
  void Advance();
  std::size_t CurrentLine() const {
    return line_;
  }
  std::size_t CurrentColumn() const {
    return column_;
  }

  Token LexIdentifierOrKeyword();
  Token LexNumber();
  Token LexString();
  Token LexCharLiteral();
  Token LexLongString();
  Token LexCodeBlock();
  Token LexSymbol();

  bool IsIdentifierStart(char c) const;
  bool IsIdentifierPart(char c) const;

  std::string source_;
  std::string file_name_;
  Diagnostics* diagnostics_ = nullptr;

  std::size_t offset_ = 0;
  std::size_t line_start_offset_ = 0;
  std::size_t line_ = 0;
  std::size_t column_ = 0;
  bool at_line_start_ = true;

  std::deque<Token> buffer_;
};

}

#endif
