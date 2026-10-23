#ifndef TIECODEC_CORE_TEXT_UTIL_H
#define TIECODEC_CORE_TEXT_UTIL_H

#include <string>
#include <vector>

namespace tiecodec {

std::string TrimRight(const std::string& text);

std::string Trim(const std::string& text);

std::vector<std::string> SplitLines(const std::string& text);

void Indent(int level, std::string* out);

}  // namespace tiecodec

#endif
