#include "file_search.h"

namespace tiecodec {
namespace file_search {
namespace {

std::filesystem::path RelativeTail(const std::filesystem::path& path) {
  std::filesystem::path tail;
  bool skipping = true;
  for (const std::filesystem::path& part : path) {
    if (skipping && part == "..")
      continue;
    skipping = false;
    if (tail.empty())
      tail = part;
    else
      tail /= part;
  }
  return tail;
}

}

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
  return !ResolvePath(raw_path, file, prefixes, found_locally).empty();
}

std::filesystem::path ResolvePath(const std::string& raw_path, const std::string& file,
                                  const std::vector<std::string>& prefixes, bool* found_locally) {
  if (raw_path.empty())
    return raw_path;
  const std::filesystem::path path(raw_path);
  const auto hit = [&found_locally](const std::filesystem::path& candidate) {
    std::error_code error;
    const std::filesystem::path canonical = std::filesystem::weakly_canonical(candidate, error);
    if (found_locally != nullptr)
      *found_locally = true;
    return error ? candidate.lexically_normal() : canonical;
  };

  if (path.is_absolute()) {
    if (PathExists(path))
      return hit(path);
    return {};
  }
  if (PathExists(BaseDirectory(file) / path))
    return hit(BaseDirectory(file) / path);
  if (PathExists(path))
    return hit(path);
  for (const std::filesystem::path& ancestor : AncestorDirectories(file)) {
    if (PathExists(ancestor / path))
      return hit(ancestor / path);
  }
  for (const std::string& prefix : prefixes) {
    if (prefix.empty())
      continue;
    const std::filesystem::path candidate = std::filesystem::path(prefix) / path;
    if (PathExists(candidate))
      return hit(candidate);
  }
  const std::filesystem::path tail = RelativeTail(path);
  if (tail != path) {
    if (PathExists(BaseDirectory(file) / tail))
      return hit(BaseDirectory(file) / tail);
    for (const std::filesystem::path& ancestor : AncestorDirectories(file)) {
      if (PathExists(ancestor / tail))
        return hit(ancestor / tail);
    }
    for (const std::string& prefix : prefixes) {
      if (prefix.empty())
        continue;
      const std::filesystem::path candidate = std::filesystem::path(prefix) / tail;
      if (PathExists(candidate))
        return hit(candidate);
    }
  }
  return {};
}

}
}
