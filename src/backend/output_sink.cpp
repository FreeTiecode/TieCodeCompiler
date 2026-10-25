#include "output_sink.h"

#include <filesystem>
#include <fstream>

namespace tiecodec {
namespace {

bool IsContained(const std::filesystem::path& root, const std::filesystem::path& target) {
  auto root_it = root.begin();
  auto target_it = target.begin();
  while (root_it != root.end()) {
    if (target_it == target.end() || *target_it != *root_it)
      return false;
    ++root_it;
    ++target_it;
  }
  return true;
}

bool ResolveTarget(const std::filesystem::path& canonical_root, const std::string& relative_path,
                   std::filesystem::path* target) {
  if (canonical_root.empty())
    return false;
  std::error_code error;
  const std::filesystem::path candidate =
      std::filesystem::weakly_canonical(canonical_root / relative_path, error);
  if (error || !IsContained(canonical_root, candidate))
    return false;
  *target = candidate;
  return true;
}

}

FileSystemOutputSink::FileSystemOutputSink(std::string root) : root_(std::move(root)) {
  std::error_code error;
  const std::filesystem::path absolute_root = std::filesystem::absolute(root_, error);
  if (!error)
    canonical_root_ = std::filesystem::weakly_canonical(absolute_root, error);
  if (error)
    canonical_root_.clear();
}

bool FileSystemOutputSink::Write(const std::string& relative_path, const std::string& content) {
  std::filesystem::path path;
  if (!ResolveTarget(canonical_root_, relative_path, &path))
    return false;
  std::error_code error;
  std::filesystem::create_directories(path.parent_path(), error);
  std::ofstream output(path, std::ios::binary);
  if (!output.is_open())
    return false;
  output.write(content.data(), static_cast<std::streamsize>(content.size()));
  return output.good();
}

bool FileSystemOutputSink::CopyFile(const std::string& source_path,
                                    const std::string& relative_path) {
  std::filesystem::path path;
  if (!ResolveTarget(canonical_root_, relative_path, &path))
    return false;
  std::error_code error;
  std::filesystem::create_directories(path.parent_path(), error);
  std::filesystem::copy_file(source_path, path, std::filesystem::copy_options::overwrite_existing,
                             error);
  return !error;
}

void FileSystemOutputSink::CreateDirectory(const std::string& relative_path) {
  std::filesystem::path path;
  if (!ResolveTarget(canonical_root_, relative_path, &path))
    return;
  std::error_code error;
  std::filesystem::create_directories(path, error);
}

}
