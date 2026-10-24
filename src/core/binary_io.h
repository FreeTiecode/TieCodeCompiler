#ifndef TIECODEC_CORE_BINARY_IO_H
#define TIECODEC_CORE_BINARY_IO_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace tiecodec {
namespace binary {

inline constexpr std::size_t kU32Size = 4;
inline constexpr std::size_t kU64Size = 8;
inline constexpr unsigned kBitsPerByte = 8;

inline void AppendU32(std::string* out, std::uint32_t value) {
  for (std::size_t i = 0; i < kU32Size; ++i) {
    out->push_back(static_cast<char>((value >> (i * kBitsPerByte)) & 0xFFu));
  }
}

inline void AppendU64(std::string* out, std::uint64_t value) {
  for (std::size_t i = 0; i < kU64Size; ++i) {
    out->push_back(static_cast<char>((value >> (i * kBitsPerByte)) & 0xFFu));
  }
}

inline bool ReadU32(const std::string& data, std::size_t* position, std::uint32_t* value) {
  if (*position + kU32Size > data.size())
    return false;
  std::uint32_t result = 0;
  for (std::size_t i = 0; i < kU32Size; ++i) {
    result |= static_cast<std::uint32_t>(static_cast<unsigned char>(data[*position + i]))
              << (i * kBitsPerByte);
  }
  *position += kU32Size;
  *value = result;
  return true;
}

inline bool ReadU64(const std::string& data, std::size_t* position, std::uint64_t* value) {
  if (*position + kU64Size > data.size())
    return false;
  std::uint64_t result = 0;
  for (std::size_t i = 0; i < kU64Size; ++i) {
    result |= static_cast<std::uint64_t>(static_cast<unsigned char>(data[*position + i]))
              << (i * kBitsPerByte);
  }
  *position += kU64Size;
  *value = result;
  return true;
}

inline bool ReadString(const std::string& data, std::size_t* position, std::string* value) {
  std::uint32_t length = 0;
  if (!ReadU32(data, position, &length))
    return false;
  if (*position + length > data.size())
    return false;
  value->assign(data, *position, length);
  *position += length;
  return true;
}

inline void AppendString(std::string* out, const std::string& value) {
  AppendU32(out, static_cast<std::uint32_t>(value.size()));
  *out += value;
}

}
}

#endif
