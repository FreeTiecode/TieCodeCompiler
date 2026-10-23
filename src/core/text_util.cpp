#include "text_util.h"

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

void Indent(int level, std::string* out) {
  out->append(static_cast<std::size_t>(level) * 2, ' ');
}

}  // namespace tiecodec
