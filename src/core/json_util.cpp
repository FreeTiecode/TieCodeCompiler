#include "json_util.h"

#include <cstdio>

namespace tiecodec {

std::string JsonEscape(const std::string& value) {
  std::string result;
  result.reserve(value.size());
  for (unsigned char c : value) {
    switch (c) {
      case '"':
        result += "\\\"";
        break;
      case '\\':
        result += "\\\\";
        break;
      case '\b':
        result += "\\b";
        break;
      case '\f':
        result += "\\f";
        break;
      case '\n':
        result += "\\n";
        break;
      case '\r':
        result += "\\r";
        break;
      case '\t':
        result += "\\t";
        break;
      default:
        if (c < 0x20) {
          char buffer[7];
          std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
          result += buffer;
        } else {
          result.push_back(static_cast<char>(c));
        }
        break;
    }
  }
  return result;
}

}
