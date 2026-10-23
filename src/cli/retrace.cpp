#include "retrace.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "line_map.h"

namespace tiecodec {
namespace {

struct BatchItem {
  std::string file;
  std::uint64_t line = 0;
};

std::string ReadAll(const std::string& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open())
    return {};
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

std::string JsonEscape(const std::string& text) {
  std::string result;
  for (char c : text) {
    switch (c) {
      case '"':
        result += "\\\"";
        break;
      case '\\':
        result += "\\\\";
        break;
      case '\n':
        result += "\\n";
        break;
      case '\t':
        result += "\\t";
        break;
      case '\r':
        result += "\\r";
        break;
      default:
        result.push_back(c);
        break;
    }
  }
  return result;
}

std::string FindStringMember(const std::string& text, const std::string& key) {
  const std::string quoted = "\"" + key + "\"";
  std::size_t position = text.find(quoted);
  if (position == std::string::npos)
    return {};
  position = text.find(':', position + quoted.size());
  if (position == std::string::npos)
    return {};
  ++position;
  while (position < text.size() && (text[position] == ' ' || text[position] == '\n'))
    ++position;
  if (position >= text.size() || text[position] != '"')
    return {};
  ++position;
  std::string value;
  while (position < text.size() && text[position] != '"') {
    if (text[position] == '\\' && position + 1 < text.size())
      ++position;
    value.push_back(text[position++]);
  }
  return value;
}

std::uint64_t FindNumberMember(const std::string& text, const std::string& key) {
  const std::string quoted = "\"" + key + "\"";
  std::size_t position = text.find(quoted);
  if (position == std::string::npos)
    return 0;
  position = text.find(':', position + quoted.size());
  if (position == std::string::npos)
    return 0;
  ++position;
  while (position < text.size() && (text[position] == ' ' || text[position] == '\n'))
    ++position;
  std::uint64_t value = 0;
  while (position < text.size() && text[position] >= '0' && text[position] <= '9') {
    value = value * 10 + static_cast<std::uint64_t>(text[position] - '0');
    ++position;
  }
  return value;
}

}  // namespace

int RunRetrace(const RetraceRequest& request) {
  SourceMapping mapping;
  std::string error;
  if (!ReadSourceMapping(request.mapping_path, &mapping, &error)) {
    std::cerr << "错误: " << error << "\n";
    return 1;
  }
  std::uint64_t source_line = 0;
  std::string source_file;
  const std::uint64_t line = std::strtoull(request.line.c_str(), nullptr, 10);
  if (RetraceLine(mapping, request.file, line, &source_file, &source_line)) {
    std::cout << source_file << ":" << source_line << "\n";
  } else {
    std::cout << ":0\n";
  }
  return 0;
}

int RunRetraceBatch(const std::string& batch_path) {
  const std::string text = ReadAll(batch_path);
  if (text.empty()) {
    std::cerr << "错误: 无法读取批量行号还原文件\n";
    return 1;
  }
  const std::string mapping_path = FindStringMember(text, "mapping");
  if (mapping_path.empty()) {
    std::cerr << "错误: 批量行号还原JSON缺少 mapping 字段\n";
    return 1;
  }
  const std::size_t items_pos = text.find("\"items\"");
  if (items_pos == std::string::npos) {
    std::cerr << "错误: 批量行号还原JSON缺少 items 数组\n";
    return 1;
  }
  std::vector<BatchItem> items;
  const std::size_t array_begin = text.find('[', items_pos);
  const std::size_t array_end = text.rfind(']');
  if (array_begin == std::string::npos || array_end == std::string::npos ||
      array_end < array_begin) {
    std::cerr << "错误: 批量行号还原JSON缺少 items 数组\n";
    return 1;
  }
  std::size_t position = array_begin + 1;
  while (position < array_end) {
    const std::size_t object_begin = text.find('{', position);
    if (object_begin == std::string::npos || object_begin >= array_end)
      break;
    const std::size_t object_end = text.find('}', object_begin);
    if (object_end == std::string::npos || object_end > array_end)
      break;
    const std::string object = text.substr(object_begin, object_end - object_begin + 1);
    BatchItem item;
    item.file = FindStringMember(object, "file");
    item.line = FindNumberMember(object, "line");
    items.push_back(std::move(item));
    position = object_end + 1;
  }
  SourceMapping mapping;
  std::string error;
  if (!ReadSourceMapping(mapping_path, &mapping, &error)) {
    std::cerr << "错误: " << error << "\n";
    return 1;
  }
  std::string output = "{\n  \"items\": [";
  for (std::size_t i = 0; i < items.size(); ++i) {
    const BatchItem& item = items[i];
    std::string source_file;
    std::uint64_t source_line = 0;
    const bool found = RetraceLine(mapping, item.file, item.line, &source_file, &source_line);
    output += i == 0 ? "\n" : ",\n";
    output += "    {\n";
    output += "      \"file\": \"" + JsonEscape(item.file) + "\",\n";
    output += std::string("      \"found\": ") + (found ? "true" : "false") + ",\n";
    output += "      \"index\": " + std::to_string(i) + ",\n";
    output += "      \"line\": " + std::to_string(item.line) + ",\n";
    output += "      \"lookupFile\": \"" + JsonEscape(item.file) + "\",\n";
    output +=
        "      \"sourceFile\": \"" + JsonEscape(found ? source_file : std::string()) + "\",\n";
    output += "      \"sourceLine\": " + std::to_string(found ? source_line : 0) + "\n";
    output += "    }";
  }
  output += items.empty() ? "],\n" : "\n  ],\n";
  output += "  \"mapping\": \"" + JsonEscape(mapping_path) + "\"\n}";
  std::cout << output << "\n";
  return 0;
}

}  // namespace tiecodec
