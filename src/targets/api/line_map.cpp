#include "line_map.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <string>

#include "binary_io.h"
#include "target.h"

namespace tiecodec {
namespace {

constexpr char kMagic[7] = {'T', 'S', 'M', 1, 0, 0, 0};
constexpr std::size_t kMagicSize = sizeof(kMagic);
constexpr std::size_t kMinimumFileSize = kMagicSize + binary::kU32Size;

}

bool WriteSourceMapping(const std::string& path, const SourceMapping& mapping, std::string* error) {
  std::string data(kMagic, kMagicSize);
  binary::AppendU32(&data, static_cast<std::uint32_t>(mapping.source_files.size()));
  for (std::size_t i = 0; i < mapping.source_files.size(); ++i) {
    binary::AppendU32(&data, static_cast<std::uint32_t>(i));
    binary::AppendU32(&data, 0);
    binary::AppendU32(&data, static_cast<std::uint32_t>(mapping.source_files[i].size()));
    data += mapping.source_files[i];
  }
  binary::AppendU32(&data, static_cast<std::uint32_t>(mapping.java_files.size()));
  for (const JavaFileMapping& file : mapping.java_files) {
    binary::AppendU32(&data, static_cast<std::uint32_t>(file.name.size()));
    data += file.name;
    binary::AppendU32(&data, static_cast<std::uint32_t>(file.entries.size()));
    for (const JavaLineEntry& entry : file.entries) {
      binary::AppendU64(&data, entry.java_line);
      binary::AppendU64(&data, entry.source_index);
      binary::AppendU64(&data, entry.source_line);
    }
  }
  binary::AppendU32(&data, static_cast<std::uint32_t>(mapping.names.size()));
  for (const auto& name : mapping.names) {
    binary::AppendU32(&data, static_cast<std::uint32_t>(name.first.size()));
    data += name.first;
    binary::AppendU32(&data, static_cast<std::uint32_t>(name.second.size()));
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
  if (data.size() < kMinimumFileSize || std::memcmp(data.data(), kMagic, kMagicSize) != 0) {
    if (error != nullptr)
      *error = "行号表格式无效: " + path;
    return false;
  }
  std::size_t pos = kMagicSize;
  std::uint32_t file_count = 0;
  if (!binary::ReadU32(data, &pos, &file_count))
    return false;
  mapping->source_files.clear();
  for (std::uint32_t i = 0; i < file_count; ++i) {
    std::uint32_t index = 0;
    std::uint32_t unused = 0;
    std::string file;
    if (!binary::ReadU32(data, &pos, &index) || !binary::ReadU32(data, &pos, &unused) ||
        !binary::ReadString(data, &pos, &file)) {
      return false;
    }
    (void)index;
    mapping->source_files.push_back(std::move(file));
  }
  std::uint32_t java_file_count = 0;
  if (!binary::ReadU32(data, &pos, &java_file_count))
    return false;
  mapping->java_files.clear();
  for (std::uint32_t i = 0; i < java_file_count; ++i) {
    JavaFileMapping file;
    if (!binary::ReadString(data, &pos, &file.name))
      return false;
    std::uint32_t entry_count = 0;
    if (!binary::ReadU32(data, &pos, &entry_count))
      return false;
    file.entries.resize(entry_count);
    for (std::uint32_t entry = 0; entry < entry_count; ++entry) {
      if (!binary::ReadU64(data, &pos, &file.entries[entry].java_line) ||
          !binary::ReadU64(data, &pos, &file.entries[entry].source_index) ||
          !binary::ReadU64(data, &pos, &file.entries[entry].source_line)) {
        return false;
      }
    }
    mapping->java_files.push_back(std::move(file));
  }
  std::uint32_t name_count = 0;
  if (!binary::ReadU32(data, &pos, &name_count))
    return false;
  mapping->names.clear();
  for (std::uint32_t i = 0; i < name_count; ++i) {
    std::string source_name;
    std::string output_name;
    if (!binary::ReadString(data, &pos, &source_name) ||
        !binary::ReadString(data, &pos, &output_name)) {
      return false;
    }
    mapping->names.emplace_back(std::move(source_name), std::move(output_name));
  }
  return true;
}

SourceMapping BuildSourceMapping(const GeneratedProgram& program) {
  SourceMapping mapping;
  mapping.names = program.mapping_names;
  std::map<std::string, std::uint64_t> file_index;
  for (const GeneratedSource& source : program.sources) {
    if (source.line_refs.empty())
      continue;
    JavaFileMapping file;
    file.name = source.file_name;
    for (std::size_t i = 0; i < source.line_refs.size(); ++i) {
      if (i == 0)
        continue;
      const LineRef& ref = source.line_refs[i];
      if (ref.line <= 0 || ref.file.empty())
        continue;
      auto index = file_index.find(ref.file);
      if (index == file_index.end()) {
        const std::uint64_t next = static_cast<std::uint64_t>(mapping.source_files.size());
        mapping.source_files.push_back(ref.file);
        index = file_index.emplace(ref.file, next).first;
      }
      JavaLineEntry entry;
      entry.java_line = static_cast<std::uint64_t>(i + 1);
      entry.source_index = index->second;
      entry.source_line = static_cast<std::uint64_t>(ref.line);
      file.entries.push_back(entry);
    }
    mapping.java_files.push_back(std::move(file));
  }
  return mapping;
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

}
