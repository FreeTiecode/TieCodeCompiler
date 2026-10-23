#ifndef TIECODEC_BACKEND_JAVA_PINYIN_TABLE_H
#define TIECODEC_BACKEND_JAVA_PINYIN_TABLE_H

#include <cstdint>
#include <string>

namespace tiecodec {

/// Pinyin of one CJK code point, or nullptr when unknown.
const char* PinyinFor(char32_t codepoint);

/// Convert a source identifier to its resource name form: pinyin tokens joined
/// with underscores, ASCII runs kept, uppercase letters start a new token.
std::string ResourceNameOf(const std::string& identifier);

/// Stable 32-bit hash used to disambiguate colliding resource names.
std::uint32_t ResourceNameHash(const std::string& text);

}  // namespace tiecodec

#endif  // TIECODEC_BACKEND_JAVA_PINYIN_TABLE_H
