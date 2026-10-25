#include "source.h"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace tiecodec {

bool FileSystemSourceProvider::ScanDirectory(const std::string& directory,
                                             std::vector<std::string>* paths, std::string* error) {
  std::error_code ec;
  std::filesystem::recursive_directory_iterator iterator(directory, ec);
  if (ec) {
    if (error != nullptr) {
      *error = "无法扫描目录: " + directory + " (" + ec.message() + ")";
    }
    return false;
  }
  const std::filesystem::recursive_directory_iterator end;
  for (; iterator != end; iterator.increment(ec)) {
    if (ec) {
      if (error != nullptr)
        *error = "扫描目录失败: " + directory + " (" + ec.message() + ")";
      return false;
    }
    if (iterator->path().extension() != ".t")
      continue;
    std::error_code file_ec;
    if (!iterator->is_regular_file(file_ec))
      continue;
    paths->push_back(iterator->path().string());
  }
  return true;
}

bool FileSystemSourceProvider::ReadFile(const std::string& path, SourceFile* file,
                                        std::string* error) {
  errno = 0;
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open()) {
    if (error != nullptr) {
      *error = "无法读取源文件: " + path;
      if (errno != 0) {
        *error += " (";
        *error += std::strerror(errno);
        *error += ")";
      }
    }
    return false;
  }
  std::ostringstream buffer;
  buffer << input.rdbuf();
  if (file != nullptr) {
    file->path = path;
    file->name = path;
    file->content = buffer.str();
  }
  return true;
}

}
