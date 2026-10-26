#ifndef TIECODEC_TARGETS_API_LINE_MAP_H
#define TIECODEC_TARGETS_API_LINE_MAP_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "emitted_text.h"
#include "target.h"

namespace tiecodec {

struct GeneratedProgram;

struct JavaLineEntry {
  std::uint64_t java_line = 0;
  std::uint64_t source_index = 0;
  std::uint64_t source_line = 0;
};

struct JavaFileMapping {
  std::string name;
  std::vector<JavaLineEntry> entries;
};

struct SourceMapping {
  std::vector<std::string> source_files;
  std::vector<JavaFileMapping> java_files;
  std::vector<std::pair<std::string, std::string>> names;
};

SourceMapping BuildSourceMapping(const GeneratedProgram& program);

bool WriteSourceMapping(const std::string& path, const SourceMapping& mapping, std::string* error);

bool ReadSourceMapping(const std::string& path, SourceMapping* mapping, std::string* error);

bool RetraceLine(const SourceMapping& mapping, const std::string& java_file, std::uint64_t line,
                 std::string* source_file, std::uint64_t* source_line);

}

#endif
