#ifndef TIECODEC_CORE_FILE_SEARCH_H
#define TIECODEC_CORE_FILE_SEARCH_H

#include <filesystem>
#include <string>
#include <vector>

#include "constants.h"

namespace tiecodec {
namespace file_search {

std::filesystem::path BaseDirectory(const std::string& file);

bool PathExists(const std::filesystem::path& path);

std::vector<std::filesystem::path> AncestorDirectories(const std::string& file,
                                                       int max_depth = kMaxAncestorSearchDepth);

bool ResolveWithPrefixes(const std::string& raw_path, const std::string& file,
                         const std::vector<std::string>& prefixes, bool* found_locally = nullptr);

std::filesystem::path ResolvePath(const std::string& raw_path, const std::string& file,
                                  const std::vector<std::string>& prefixes,
                                  bool* found_locally = nullptr);

}
}

#endif
