#ifndef TIECODEC_BACKEND_JAVA_LINE_MAP_H
#define TIECODEC_BACKEND_JAVA_LINE_MAP_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "java_emitter.h"

namespace tiecodec {

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

/// Write the TSM line map file (format compatible with the reference tooling).
bool WriteSourceMapping(const std::string& path, const SourceMapping& mapping, std::string* error);

/// Read a TSM line map file.
bool ReadSourceMapping(const std::string& path, SourceMapping* mapping, std::string* error);

/// Resolve one output line; returns false when the line has no mapping.
bool RetraceLine(const SourceMapping& mapping, const std::string& java_file, std::uint64_t line,
                 std::string* source_file, std::uint64_t* source_line);

}  // namespace tiecodec

#endif  // TIECODEC_BACKEND_JAVA_LINE_MAP_H
