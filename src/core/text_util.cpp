#include "text_util.h"

#include "constants.h"

namespace tiecodec {

std::string TrimRight(const std::string& text) {
  std::size_t end = text.size();
  while (end > 0 && (text[end - 1] == ' ' || text[end - 1] == '\t' || text[end - 1] == '\r')) {
    --end;
  }
  return text.substr(0, end);
}

std::string Trim(const std::string& text) {
  std::size_t begin = 0;
  while (begin < text.size() &&
         (text[begin] == ' ' || text[begin] == '\t' || text[begin] == '\r')) {
    ++begin;
  }
  return TrimRight(text.substr(begin));
}

std::vector<std::string> SplitLines(const std::string& text) {
  std::vector<std::string> lines;
  std::string current;
  for (char c : text) {
    if (c == '\n') {
      lines.push_back(current);
      current.clear();
    } else {
      current.push_back(c);
    }
  }
  if (!current.empty())
    lines.push_back(current);
  return lines;
}

std::vector<std::string> SplitDelimited(const std::string& text, char delimiter, bool keep_empty) {
  std::vector<std::string> parts;
  std::size_t begin = 0;
  while (begin <= text.size()) {
    const std::size_t end = text.find(delimiter, begin);
    const std::string part =
        text.substr(begin, end == std::string::npos ? std::string::npos : end - begin);
    if (keep_empty || !part.empty())
      parts.push_back(part);
    if (end == std::string::npos)
      break;
    begin = end + 1;
  }
  return parts;
}

std::string JoinStrings(const std::vector<std::string>& items, const std::string& separator) {
  std::string result;
  for (const std::string& item : items) {
    if (!result.empty())
      result += separator;
    result += item;
  }
  return result;
}

bool EndsWith(const std::string& text, const std::string& suffix) {
  return text.size() >= suffix.size() &&
         text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool IsQuoted(const std::string& text) {
  return text.size() >= 2 && ((text.front() == '"' && text.back() == '"') ||
                              (text.front() == '\'' && text.back() == '\''));
}

std::string Unquote(const std::string& text) {
  return IsQuoted(text) ? text.substr(1, text.size() - 2) : text;
}

std::string UnwrapQuotes(const std::string& text) {
  if (IsQuoted(text))
    return text.substr(1, text.size() - 2);
  if (text.size() >= 2 && (text.front() == '"' || text.front() == '\''))
    return text.substr(1);
  return text;
}

std::string Unescape(const std::string& text) {
  std::string result;
  result.reserve(text.size());
  for (std::size_t index = 0; index < text.size(); ++index) {
    if (text[index] != '\\' || index + 1 >= text.size()) {
      result.push_back(text[index]);
      continue;
    }
    const char escaped = text[++index];
    switch (escaped) {
      case 'n':
        result.push_back('\n');
        break;
      case 't':
        result.push_back('\t');
        break;
      case 'r':
        result.push_back('\r');
        break;
      case '"':
        result.push_back('"');
        break;
      case '\'':
        result.push_back('\'');
        break;
      case '\\':
        result.push_back('\\');
        break;
      default:
        result.push_back(escaped);
        break;
    }
  }
  return result;
}

void Indent(int level, std::string* out) {
  out->append(static_cast<std::size_t>(level) * kIndentWidth, ' ');
}

std::string IndentText(int level) {
  return std::string(static_cast<std::size_t>(level) * kIndentWidth, ' ');
}

std::string Spaces(int count) {
  return std::string(static_cast<std::size_t>(count), ' ');
}

}
