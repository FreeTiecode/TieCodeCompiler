#ifndef TIECODEC_CORE_TEXT_UTIL_H
#define TIECODEC_CORE_TEXT_UTIL_H

#include <string>
#include <vector>

namespace tiecodec {

std::string TrimRight(const std::string& text);

std::string Trim(const std::string& text);

std::vector<std::string> SplitLines(const std::string& text);

std::vector<std::string> SplitDelimited(const std::string& text, char delimiter,
                                        bool keep_empty = false);

std::string JoinStrings(const std::vector<std::string>& items, const std::string& separator);

bool EndsWith(const std::string& text, const std::string& suffix);

bool IsQuoted(const std::string& text);

std::string Unquote(const std::string& text);

std::string UnwrapQuotes(const std::string& text);

std::string Unescape(const std::string& text);

void Indent(int level, std::string* out);

std::string IndentText(int level);

std::string Spaces(int count);

}

#endif
