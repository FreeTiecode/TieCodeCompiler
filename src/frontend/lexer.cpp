#include "lexer.h"

#include <cctype>

#include "constants.h"

namespace tiecodec {
namespace {

bool IsAsciiDigit(char c) {
  return c >= '0' && c <= '9';
}

bool IsAsciiAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool IsContinuationByte(char c) {
  return (static_cast<unsigned char>(c) & 0xC0) == 0x80;
}

bool IsIdentifierAscii(char c) {
  return IsAsciiAlpha(c) || IsAsciiDigit(c) || c == '_' || c == '$';
}

}

const char* TokenKindName(TokenKind kind) {
  switch (kind) {
    case TokenKind::kEnd:
      return "end of file";
    case TokenKind::kIdentifier:
      return "identifier";
    case TokenKind::kInteger:
      return "integer";
    case TokenKind::kLong:
      return "long";
    case TokenKind::kFloat:
      return "float";
    case TokenKind::kDouble:
      return "double";
    case TokenKind::kChar:
      return "char";
    case TokenKind::kString:
      return "string";
    case TokenKind::kLongString:
      return "long string";
    case TokenKind::kCodeBlock:
      return "code block";
    case TokenKind::kSymbol:
      return "symbol";
  }
  return "token";
}

Lexer::Lexer(std::string source, std::string file_name, Diagnostics* diagnostics)
    : source_(std::move(source)), file_name_(std::move(file_name)), diagnostics_(diagnostics) {
  if (source_.size() >= 3 && static_cast<unsigned char>(source_[0]) == 0xEF &&
      static_cast<unsigned char>(source_[1]) == 0xBB &&
      static_cast<unsigned char>(source_[2]) == 0xBF) {
    offset_ = 3;
  }
}

const Token& Lexer::Peek(std::size_t count) {
  while (buffer_.size() <= count) {
    buffer_.push_back(Lex());
  }
  return buffer_[count];
}

Token Lexer::Next() {
  if (!buffer_.empty()) {
    Token token = std::move(buffer_.front());
    buffer_.pop_front();
    return token;
  }
  return Lex();
}

char Lexer::Current() const {
  return offset_ < source_.size() ? source_[offset_] : '\0';
}

char Lexer::PeekChar(std::size_t offset) const {
  const std::size_t index = offset_ + offset;
  return index < source_.size() ? source_[index] : '\0';
}

bool Lexer::AtEnd() const {
  return offset_ >= source_.size();
}

void Lexer::Advance() {
  if (AtEnd())
    return;
  const char c = source_[offset_];
  ++offset_;
  if (c == '\n') {
    ++line_;
    column_ = 0;
    line_start_offset_ = offset_;
    at_line_start_ = true;
  } else if (!IsContinuationByte(c)) {
    if (c == '\t') {
      column_ += kTabWidth - (column_ % kTabWidth);
    } else {
      ++column_;
    }
    if (c != ' ' && c != '\t' && c != '\r')
      at_line_start_ = false;
  }
}

bool Lexer::IsIdentifierStart(char c) const {
  return IsIdentifierAscii(c) || static_cast<unsigned char>(c) >= 0x80;
}

bool Lexer::IsIdentifierPart(char c) const {
  return IsIdentifierStart(c);
}

void Lexer::SkipWhitespaceAndComments() {
  while (!AtEnd()) {
    const char c = Current();
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
      Advance();
      continue;
    }
    if (c == '/' && PeekChar(1) == '/') {
      while (!AtEnd() && Current() != '\n')
        Advance();
      continue;
    }
    if (c == '/' && PeekChar(1) == '*') {
      Advance();
      Advance();
      while (!AtEnd()) {
        if (Current() == '*' && PeekChar(1) == '/') {
          Advance();
          Advance();
          break;
        }
        Advance();
      }
      continue;
    }
    break;
  }
}

void Lexer::ReportError(const std::string& message) {
  if (diagnostics_ == nullptr)
    return;
  SourcePosition position;
  position.line = line_;
  position.column = column_;
  position.offset = offset_;
  diagnostics_->Error(file_name_, position, message, "lexer.error");
}

int Lexer::IndentLevelAt(std::size_t line_start, std::size_t offset) const {
  int tabs = 0;
  int spaces = 0;
  for (std::size_t i = line_start; i < offset && i < source_.size(); ++i) {
    const char c = source_[i];
    if (c == '\t') {
      ++tabs;
    } else if (c == ' ') {
      ++spaces;
    } else {
      break;
    }
  }
  return tabs + spaces / kTabWidth;
}

Token Lexer::MakeToken(TokenKind kind, std::size_t begin_offset, std::size_t begin_line,
                       std::size_t begin_column) const {
  Token token;
  token.kind = kind;
  token.position.line = begin_line;
  token.position.column = begin_column;
  token.position.offset = begin_offset;
  token.begins_line = begin_column == 0;
  token.indent_level = IndentLevelAt(line_start_offset_, begin_offset);
  return token;
}

Token Lexer::Lex() {
  SkipWhitespaceAndComments();

  const std::size_t begin_offset = offset_;
  const std::size_t begin_line = line_;
  const std::size_t begin_column = column_;

  if (AtEnd()) {
    Token token = MakeToken(TokenKind::kEnd, begin_offset, begin_line, begin_column);
    return token;
  }

  const char c = Current();

  if (c == '@' && source_.compare(offset_, 5, "@code") == 0) {
    const char next = PeekChar(5);
    if (next == '\0' || next == ' ' || next == '\t' || next == '\r' || next == '\n' ||
        next == '(') {
      return LexCodeBlock();
    }
  }

  if (AtEnd()) {
    return MakeToken(TokenKind::kEnd, begin_offset, begin_line, begin_column);
  }

  if (IsIdentifierStart(Current()) && !IsAsciiDigit(Current())) {
    if (begin_line == line_ && source_.compare(offset_, 4, "code") == 0) {
      const char next = PeekChar(4);
      const bool is_code_keyword =
          next == '\0' || next == ' ' || next == '\t' || next == '\r' || next == '\n';
      if (is_code_keyword && source_.compare(offset_, 5, "code(") != 0) {
        Token token = MakeToken(TokenKind::kCodeBlock, begin_offset, begin_line, begin_column);
        token.raw = source_.substr(offset_, 4);
        for (int i = 0; i < 4; ++i)
          Advance();
        token.text = ReadRestOfLine();
        return token;
      }
    }
    return LexIdentifierOrKeyword();
  }
  if (IsAsciiDigit(c))
    return LexNumber();
  if (c == '"')
    return LexString();
  if (c == '\'')
    return LexCharLiteral();
  if (c == '[' && PeekChar(1) == '[')
    return LexLongString();
  return LexSymbol();
}

Token Lexer::LexIdentifierOrKeyword() {
  Token token = MakeToken(TokenKind::kIdentifier, offset_, line_, column_);
  while (!AtEnd() && IsIdentifierPart(Current()))
    Advance();
  token.text = source_.substr(token.position.offset, offset_ - token.position.offset);
  token.raw = token.text;
  return token;
}

Token Lexer::LexNumber() {
  const std::size_t begin = offset_;
  const std::size_t begin_line = line_;
  const std::size_t begin_column = column_;
  bool is_decimal = false;
  bool is_hex = false;

  if (Current() == '0' && (PeekChar(1) == 'x' || PeekChar(1) == 'X')) {
    is_hex = true;
    Advance();
    Advance();
    while (!AtEnd() && std::isxdigit(static_cast<unsigned char>(Current())))
      Advance();
  } else {
    while (!AtEnd() && IsAsciiDigit(Current()))
      Advance();
    if (Current() == '.' && IsAsciiDigit(PeekChar(1))) {
      is_decimal = true;
      Advance();
      while (!AtEnd() && IsAsciiDigit(Current()))
        Advance();
    }
    if (Current() == 'e' || Current() == 'E') {
      is_decimal = true;
      Advance();
      if (Current() == '+' || Current() == '-')
        Advance();
      while (!AtEnd() && IsAsciiDigit(Current()))
        Advance();
    }
  }

  TokenKind kind = TokenKind::kInteger;
  if (!is_hex) {
    const char suffix = Current();
    if (suffix == 'L' || suffix == 'l') {
      kind = TokenKind::kLong;
      Advance();
    } else if (suffix == 'F' || suffix == 'f') {
      kind = TokenKind::kFloat;
      Advance();
    } else if (suffix == 'D' || suffix == 'd') {
      kind = TokenKind::kDouble;
      Advance();
    } else if (is_decimal) {
      kind = TokenKind::kDouble;
    }
  }

  Token token = MakeToken(kind, begin, begin_line, begin_column);
  token.text = source_.substr(begin, offset_ - begin);
  token.raw = token.text;
  return token;
}

Token Lexer::LexString() {
  const std::size_t begin = offset_;
  const std::size_t begin_line = line_;
  const std::size_t begin_column = column_;
  Advance();
  std::string decoded;
  while (!AtEnd() && Current() != '"') {
    if (Current() == '\\') {
      Advance();
      const char escape = Current();
      switch (escape) {
        case 'n':
          decoded.push_back('\n');
          break;
        case 't':
          decoded.push_back('\t');
          break;
        case 'r':
          decoded.push_back('\r');
          break;
        case 'b':
          decoded.push_back('\b');
          break;
        case 'f':
          decoded.push_back('\f');
          break;
        case '0':
          decoded.push_back('\0');
          break;
        case '\\':
          decoded.push_back('\\');
          break;
        case '"':
          decoded.push_back('"');
          break;
        case '\'':
          decoded.push_back('\'');
          break;
        case 'u': {
          decoded.push_back('\\');
          decoded.push_back('u');
          break;
        }
        default:
          decoded.push_back(escape);
          break;
      }
      Advance();
      continue;
    }
    decoded.push_back(Current());
    Advance();
  }
  if (AtEnd())
    ReportError("未结束的字符串");
  Advance();
  Token token = MakeToken(TokenKind::kString, begin, begin_line, begin_column);
  token.text = std::move(decoded);
  token.raw = source_.substr(begin, offset_ - begin);
  return token;
}

Token Lexer::LexCharLiteral() {
  const std::size_t begin = offset_;
  const std::size_t begin_line = line_;
  const std::size_t begin_column = column_;
  Advance();
  std::string decoded;
  if (Current() == '\\') {
    Advance();
    const char escape = Current();
    switch (escape) {
      case 'n':
        decoded.push_back('\n');
        break;
      case 't':
        decoded.push_back('\t');
        break;
      case 'r':
        decoded.push_back('\r');
        break;
      case '0':
        decoded.push_back('\0');
        break;
      case '\\':
        decoded.push_back('\\');
        break;
      case '\'':
        decoded.push_back('\'');
        break;
      default:
        decoded.push_back(escape);
        break;
    }
    Advance();
  } else if (!AtEnd()) {
    decoded.push_back(Current());
    Advance();
  }
  if (Current() == '\'')
    Advance();
  else
    ReportError("未结束的字符字面量");
  Token token = MakeToken(TokenKind::kChar, begin, begin_line, begin_column);
  token.text = std::move(decoded);
  token.raw = source_.substr(begin, offset_ - begin);
  return token;
}

Token Lexer::LexLongString() {
  const std::size_t begin = offset_;
  const std::size_t begin_line = line_;
  const std::size_t begin_column = column_;
  const int start_indent = IndentLevelAt(line_start_offset_, begin);
  Advance();
  Advance();
  std::string content;
  while (!AtEnd()) {
    if (Current() == ']' && PeekChar(1) == ']') {
      Advance();
      Advance();
      break;
    }
    content.push_back(Current());
    Advance();
  }
  Token token = MakeToken(TokenKind::kLongString, begin, begin_line, begin_column);
  token.indent_level = start_indent;
  token.text = std::move(content);
  token.raw = source_.substr(begin, offset_ - begin);
  return token;
}

Token Lexer::LexCodeBlock() {
  const std::size_t begin = offset_;
  const std::size_t begin_line = line_;
  const std::size_t begin_column = column_;

  const int start_indent = IndentLevelAt(line_start_offset_, begin);
  for (int i = 0; i < 5; ++i)
    Advance();

  if (Current() == '(') {
    while (!AtEnd() && Current() != ')')
      Advance();
    if (!AtEnd())
      Advance();
  }

  while (!AtEnd() && Current() != '\n')
    Advance();
  if (!AtEnd())
    Advance();

  const std::size_t content_begin = offset_;
  std::size_t content_end = offset_;
  while (!AtEnd()) {
    std::size_t probe = offset_;
    while (probe < source_.size() && (source_[probe] == ' ' || source_[probe] == '\t'))
      ++probe;
    if (source_.compare(probe, 4, "@end") == 0) {
      const std::size_t after = probe + 4;
      const bool boundary = after >= source_.size() || source_[after] == '\n' ||
                            source_[after] == ' ' || source_[after] == '\t' ||
                            source_[after] == '\r';
      if (boundary) {
        content_end = offset_;

        while (!AtEnd() && Current() != '\n')
          Advance();
        break;
      }
    }
    Advance();
  }
  if (content_end < content_begin)
    content_end = content_begin;
  std::string raw = source_.substr(content_begin, content_end - content_begin);

  Token token = MakeToken(TokenKind::kCodeBlock, begin, begin_line, begin_column);
  token.indent_level = start_indent;
  token.raw = std::move(raw);
  token.text = token.raw;
  return token;
}

Token Lexer::LexSymbol() {
  static const char* kSymbols[] = {
      "<=>", "...", "->", "==", "!=", "<=", ">=", "&&", "||", "@", "#",
      "+",   "-",   "*",  "/",  "%",  "&",  "|",  "^",  "<",  ">", "=",
      "?",   ":",   ".",  ",",  ";",  "(",  ")",  "{",  "}",  "[", "]",
  };
  const std::size_t begin = offset_;
  const std::size_t begin_line = line_;
  const std::size_t begin_column = column_;
  for (const char* symbol : kSymbols) {
    const std::size_t length = std::char_traits<char>::length(symbol);
    if (source_.compare(offset_, length, symbol) == 0) {
      for (std::size_t i = 0; i < length; ++i)
        Advance();
      Token token = MakeToken(TokenKind::kSymbol, begin, begin_line, begin_column);
      token.text = symbol;
      token.raw = symbol;
      return token;
    }
  }

  const std::string text(1, Current());
  Advance();
  ReportError("无法识别的字符: " + text);
  Token token = MakeToken(TokenKind::kSymbol, begin, begin_line, begin_column);
  token.text = text;
  token.raw = text;
  return token;
}

std::string Lexer::ReadRestOfLine() {
  std::string text;
  while (!AtEnd() && Current() != '\n') {
    text.push_back(Current());
    Advance();
  }

  while (!text.empty() && (text.back() == ' ' || text.back() == '\t' || text.back() == '\r')) {
    text.pop_back();
  }

  std::size_t begin = 0;
  while (begin < text.size() && (text[begin] == ' ' || text[begin] == '\t'))
    ++begin;
  return text.substr(begin);
}

void Lexer::SkipRestOfLine() {
  while (!AtEnd() && Current() != '\n')
    Advance();
}

}
