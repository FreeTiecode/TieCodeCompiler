#include "file_search.h"

namespace tiecodec {
namespace file_search {

std::filesystem::path BaseDirectory(const std::string& file) {
  std::filesystem::path path(file);
  if (path.has_parent_path())
    return path.parent_path();
  return std::filesystem::path(".");
}

bool PathExists(const std::filesystem::path& path) {
  std::error_code error;
  return std::filesystem::exists(path, error);
}

std::vector<std::filesystem::path> AncestorDirectories(const std::string& file, int max_depth) {
  std::vector<std::filesystem::path> result;
  std::filesystem::path directory = BaseDirectory(file);
  for (int depth = 0; depth < max_depth; ++depth) {
    if (directory.empty() || directory == directory.root_path())
      break;
    if (!directory.has_parent_path())
      break;
    const std::filesystem::path parent = directory.parent_path();
    if (parent == directory)
      break;
    result.push_back(parent);
    directory = parent;
  }
  return result;
}

bool ResolveWithPrefixes(const std::string& raw_path, const std::string& file,
                         const std::vector<std::string>& prefixes, bool* found_locally) {
  if (raw_path.empty())
    return true;
  const std::filesystem::path path(raw_path);
  if (path.is_absolute()) {
    if (PathExists(path)) {
      if (found_locally != nullptr)
        *found_locally = true;
      return true;
    }
    return false;
  }
  if (PathExists(BaseDirectory(file) / path)) {
    if (found_locally != nullptr)
      *found_locally = true;
    return true;
  }
  if (PathExists(path)) {
    if (found_locally != nullptr)
      *found_locally = true;
    return true;
  }
  for (const std::filesystem::path& ancestor : AncestorDirectories(file)) {
    if (PathExists(ancestor / path)) {
      if (found_locally != nullptr)
        *found_locally = true;
      return true;
    }
  }
  for (const std::string& prefix : prefixes) {
    if (prefix.empty())
      continue;
    if (PathExists(std::filesystem::path(prefix) / path))
      return true;
  }
  return false;
}

}
}
