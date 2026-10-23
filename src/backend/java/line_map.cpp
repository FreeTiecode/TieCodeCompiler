#include "line_map.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <string>

namespace tiecodec {
namespace {

constexpr char kMagic[7] = {'T', 'S', 'M', 1, 0, 0, 0};

void WriteU32(std::string* out, std::uint32_t value) {
  for (int i = 0; i < 4; ++i)
    out->push_back(static_cast<char>((value >> (i * 8)) & 0xFF));
}

void WriteU64(std::string* out, std::uint64_t value) {
  for (int i = 0; i < 8; ++i)
    out->push_back(static_cast<char>((value >> (i * 8)) & 0xFF));
}

bool ReadU32(const std::string& data, std::size_t* pos, std::uint32_t* value) {
  if (*pos + 4 > data.size())
    return false;
  std::uint32_t result = 0;
  for (int i = 0; i < 4; ++i) {
    result |= static_cast<std::uint32_t>(static_cast<unsigned char>(data[*pos + i])) << (i * 8);
  }
  *pos += 4;
  *value = result;
  return true;
}

bool ReadU64(const std::string& data, std::size_t* pos, std::uint64_t* value) {
  if (*pos + 8 > data.size())
    return false;
  std::uint64_t result = 0;
  for (int i = 0; i < 8; ++i) {
    result |= static_cast<std::uint64_t>(static_cast<unsigned char>(data[*pos + i])) << (i * 8);
  }
  *pos += 8;
  *value = result;
  return true;
}

bool ReadString(const std::string& data, std::size_t* pos, std::string* value) {
  std::uint32_t length = 0;
  if (!ReadU32(data, pos, &length))
    return false;
  if (*pos + length > data.size())
    return false;
  value->assign(data, *pos, length);
  *pos += length;
  return true;
}

}  // namespace

bool WriteSourceMapping(const std::string& path, const SourceMapping& mapping, std::string* error) {
  std::string data(kMagic, 7);
  WriteU32(&data, static_cast<std::uint32_t>(mapping.source_files.size()));
  for (std::size_t i = 0; i < mapping.source_files.size(); ++i) {
    WriteU32(&data, static_cast<std::uint32_t>(i));
    WriteU32(&data, 0);
    WriteU32(&data, static_cast<std::uint32_t>(mapping.source_files[i].size()));
    data += mapping.source_files[i];
  }
  WriteU32(&data, static_cast<std::uint32_t>(mapping.java_files.size()));
  for (const JavaFileMapping& file : mapping.java_files) {
    WriteU32(&data, static_cast<std::uint32_t>(file.name.size()));
    data += file.name;
    WriteU32(&data, static_cast<std::uint32_t>(file.entries.size()));
    for (const JavaLineEntry& entry : file.entries) {
      WriteU64(&data, entry.java_line);
      WriteU64(&data, entry.source_index);
      WriteU64(&data, entry.source_line);
    }
  }
  WriteU32(&data, static_cast<std::uint32_t>(mapping.names.size()));
  for (const auto& name : mapping.names) {
    WriteU32(&data, static_cast<std::uint32_t>(name.first.size()));
    data += name.first;
    WriteU32(&data, static_cast<std::uint32_t>(name.second.size()));
    data += name.second;
  }
  std::ofstream output(path, std::ios::binary);
  if (!output.is_open()) {
    if (error != nullptr)
      *error = "无法写入行号表: " + path;
    return false;
  }
  output.write(data.data(), static_cast<std::streamsize>(data.size()));
  if (!output.good()) {
    if (error != nullptr)
      *error = "写入行号表失败: " + path;
    return false;
  }
  return true;
}

bool ReadSourceMapping(const std::string& path, SourceMapping* mapping, std::string* error) {
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open()) {
    if (error != nullptr)
      *error = "无法读取行号表: " + path;
    return false;
  }
  std::string data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
  if (data.size() < 11 || std::memcmp(data.data(), kMagic, 7) != 0) {
    if (error != nullptr)
      *error = "行号表格式无效: " + path;
    return false;
  }
  std::size_t pos = 7;
  std::uint32_t file_count = 0;
  if (!ReadU32(data, &pos, &file_count))
    return false;
  mapping->source_files.clear();
  for (std::uint32_t i = 0; i < file_count; ++i) {
    std::uint32_t index = 0;
    std::uint32_t unused = 0;
    std::string file;
    if (!ReadU32(data, &pos, &index) || !ReadU32(data, &pos, &unused) ||
        !ReadString(data, &pos, &file)) {
      return false;
    }
    (void)index;
    mapping->source_files.push_back(std::move(file));
  }
  std::uint32_t java_file_count = 0;
  if (!ReadU32(data, &pos, &java_file_count))
    return false;
  mapping->java_files.clear();
  for (std::uint32_t i = 0; i < java_file_count; ++i) {
    JavaFileMapping file;
    if (!ReadString(data, &pos, &file.name))
      return false;
    std::uint32_t entry_count = 0;
    if (!ReadU32(data, &pos, &entry_count))
      return false;
    file.entries.resize(entry_count);
    for (std::uint32_t entry = 0; entry < entry_count; ++entry) {
      if (!ReadU64(data, &pos, &file.entries[entry].java_line) ||
          !ReadU64(data, &pos, &file.entries[entry].source_index) ||
          !ReadU64(data, &pos, &file.entries[entry].source_line)) {
        return false;
      }
    }
    mapping->java_files.push_back(std::move(file));
  }
  std::uint32_t name_count = 0;
  if (!ReadU32(data, &pos, &name_count))
    return false;
  mapping->names.clear();
  for (std::uint32_t i = 0; i < name_count; ++i) {
    std::string source_name;
    std::string output_name;
    if (!ReadString(data, &pos, &source_name) || !ReadString(data, &pos, &output_name)) {
      return false;
    }
    mapping->names.emplace_back(std::move(source_name), std::move(output_name));
  }
  return true;
}

bool RetraceLine(const SourceMapping& mapping, const std::string& java_file, std::uint64_t line,
                 std::string* source_file, std::uint64_t* source_line) {
  for (const JavaFileMapping& file : mapping.java_files) {
    if (file.name != java_file)
      continue;
    for (const JavaLineEntry& entry : file.entries) {
      if (entry.java_line != line)
        continue;
      if (entry.source_index >= mapping.source_files.size())
        return false;
      *source_file = mapping.source_files[entry.source_index];
      *source_line = entry.source_line;
      return true;
    }
    return false;
  }
  return false;
}

}  // namespace tiecodec
