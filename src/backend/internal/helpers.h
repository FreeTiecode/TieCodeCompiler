#ifndef TIECODEC_BACKEND_INTERNAL_HELPERS_H
#define TIECODEC_BACKEND_INTERNAL_HELPERS_H

#include <cctype>
#include <string>
#include <utility>
#include <vector>

#include "ast.h"
#include "constants.h"
#include "text_util.h"

namespace tiecodec {
namespace detail {

inline bool TryFoldInteger(const Expr& expression, long long* value) {
  switch (expression.kind) {
    case ExprKind::kLiteral: {
      const std::string& text = expression.text;
      if (text.empty())
        return false;
      std::size_t index = 0;
      if (text[0] == '-' || text[0] == '+')
        index = 1;
      if (index >= text.size())
        return false;
      for (std::size_t i = index; i < text.size(); ++i) {
        if (text[i] < '0' || text[i] > '9')
          return false;
      }
      try {
        *value = std::stoll(text);
      } catch (...) {
        return false;
      }
      return true;
    }
    case ExprKind::kParen: {
      if (expression.children.size() != 1 || expression.children[0] == nullptr)
        return false;
      return TryFoldInteger(*expression.children[0], value);
    }
    case ExprKind::kUnary: {
      if (expression.text != "-" || expression.children.size() != 1 ||
          expression.children[0] == nullptr) {
        return false;
      }
      long long operand = 0;
      if (!TryFoldInteger(*expression.children[0], &operand))
        return false;
      *value = -operand;
      return true;
    }
    case ExprKind::kBinary: {
      if (expression.children.size() != 2 || expression.children[0] == nullptr ||
          expression.children[1] == nullptr) {
        return false;
      }
      long long left = 0;
      long long right = 0;
      if (!TryFoldInteger(*expression.children[0], &left))
        return false;
      if (!TryFoldInteger(*expression.children[1], &right))
        return false;
      const std::string& op = expression.text;
      if (op == "+")
        *value = left + right;
      else if (op == "-")
        *value = left - right;
      else if (op == "*")
        *value = left * right;
      else if (op == "/") {
        if (right == 0)
          return false;
        *value = left / right;
      } else if (op == "%") {
        if (right == 0)
          return false;
        *value = left % right;
      } else
        return false;
      return true;
    }
    default:
      return false;
  }
}

struct TypeShape {
  std::string base;
  int dimensions = 0;
  std::vector<std::string> capacities;
};

inline TypeShape ParseTypeShape(const std::string& type_name) {
  TypeShape shape;
  std::size_t i = 0;
  int generic_depth = 0;
  for (; i < type_name.size(); ++i) {
    const char c = type_name[i];
    if (c == '<')
      ++generic_depth;
    if (c == '>')
      --generic_depth;
    if (c == '[' && generic_depth == 0)
      break;
    shape.base.push_back(c);
  }
  while (i < type_name.size()) {
    if (type_name[i] != '[')
      break;
    const std::size_t close = type_name.find(']', i);
    if (close == std::string::npos)
      break;
    shape.capacities.push_back(type_name.substr(i + 1, close - i - 1));
    ++shape.dimensions;
    i = close + 1;
  }
  return shape;
}

inline int CodeLineIndentLevel(const std::string& line) {
  int tabs = 0;
  int spaces = 0;
  for (char c : line) {
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

inline std::vector<std::string> NormalizeCodeBlock(const std::string& code, int base_indent_level,
                                                   int code_indent) {
  std::vector<std::string> lines;
  for (const std::string& raw : SplitLines(code)) {
    std::size_t first = 0;
    while (first < raw.size() && (raw[first] == ' ' || raw[first] == '\t'))
      ++first;
    const std::string content = raw.substr(first);
    if (content.empty())
      continue;

    if (content[0] == '*') {
      lines.push_back(raw);
      continue;
    }
    const int relative = CodeLineIndentLevel(raw) - code_indent;
    const int level = relative > 0 ? relative : 0;
    lines.push_back(IndentText(base_indent_level + level) + content);
  }
  return lines;
}

inline const std::string& CodeTextOf(const Member& member) {
  static const std::string kEmpty;
  if (!member.code_text.empty())
    return member.code_text;
  if (member.body.size() == 1 && member.body[0] != nullptr &&
      member.body[0]->kind == StmtKind::kCode) {
    return member.body[0]->code_text;
  }
  return kEmpty;
}

inline int CodeLineOf(const Member& member) {
  if (!member.code_text.empty())
    return member.code_line;
  if (member.body.size() == 1 && member.body[0] != nullptr &&
      member.body[0]->kind == StmtKind::kCode) {
    return member.body[0]->code_line;
  }
  return member.code_line;
}

inline int CodeIndentOf(const Member& member) {
  if (!member.code_text.empty())
    return member.code_indent;
  if (member.body.size() == 1 && member.body[0] != nullptr &&
      member.body[0]->kind == StmtKind::kCode) {
    return member.body[0]->code_indent;
  }
  return member.code_indent;
}

inline bool ReadAngleContent(const std::string& code, std::size_t* index, std::string* content) {
  if (*index >= code.size() || code[*index] != '<')
    return false;
  std::size_t depth = 0;
  std::size_t begin = *index + 1;
  std::size_t i = *index;
  for (; i < code.size(); ++i) {
    if (code[i] == '<') {
      ++depth;
    } else if (code[i] == '>') {
      --depth;
      if (depth == 0) {
        *content = code.substr(begin, i - begin);
        *index = i + 1;
        return true;
      }
    }
  }
  return false;
}

inline bool IsIdentifierByte(char c) {
  const unsigned char u = static_cast<unsigned char>(c);
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' ||
         u >= 0x80;
}

}
}

#endif
