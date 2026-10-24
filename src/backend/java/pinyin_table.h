#ifndef TIECODEC_BACKEND_JAVA_PINYIN_TABLE_H
#define TIECODEC_BACKEND_JAVA_PINYIN_TABLE_H

#include <cstdint>
#include <string>

namespace tiecodec {

const char* PinyinFor(char32_t codepoint);

std::string ResourceNameOf(const std::string& identifier);

std::uint32_t ResourceNameHash(const std::string& text);

}

#endif
