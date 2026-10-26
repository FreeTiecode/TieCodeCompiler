#ifndef TIECODEC_CORE_PINYIN_TABLE_H
#define TIECODEC_CORE_PINYIN_TABLE_H

#include <cstdint>
#include <string>
#include <vector>

namespace tiecodec {

const char* PinyinFor(char32_t codepoint);

std::vector<std::string> IdentifierTokens(const std::string& identifier);

std::string CamelCase(const std::string& identifier);

std::string LowerCamelCase(const std::string& identifier);

std::string SegmentPascal(const std::string& identifier);

std::string PackageInitials(const std::string& segment);

std::string TransformPackage(const std::string& package);

std::string ResourceNameOf(const std::string& identifier);

std::uint32_t ResourceNameHash(const std::string& text);

}

#endif
