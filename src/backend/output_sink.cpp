#include "output_sink.h"

#include <filesystem>
#include <fstream>

namespace tiecodec {

bool FileSystemOutputSink::Write(const std::string& relative_path, const std::string& content) {
  const std::filesystem::path path = std::filesystem::path(root_) / relative_path;
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
  const std::filesystem::path path = std::filesystem::path(root_) / relative_path;
  std::error_code error;
  std::filesystem::create_directories(path.parent_path(), error);
  std::filesystem::copy_file(source_path, path, std::filesystem::copy_options::overwrite_existing,
                             error);
  return !error;
}

void FileSystemOutputSink::CreateDirectory(const std::string& relative_path) {
  std::error_code error;
  std::filesystem::create_directories(std::filesystem::path(root_) / relative_path, error);
}

}  // namespace tiecodec
