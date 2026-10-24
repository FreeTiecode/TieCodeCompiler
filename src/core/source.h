#ifndef TIECODEC_CORE_SOURCE_H
#define TIECODEC_CORE_SOURCE_H

#include <string>
#include <vector>

namespace tiecodec {

struct SourceFile {
  std::string name;
  std::string path;
  std::string content;
};

class SourceProvider {
public:
  virtual ~SourceProvider() = default;

  virtual bool ScanDirectory(const std::string& directory, std::vector<std::string>* paths,
                             std::string* error) = 0;

  virtual bool ReadFile(const std::string& path, SourceFile* file, std::string* error) = 0;
};

class FileSystemSourceProvider : public SourceProvider {
public:
  bool ScanDirectory(const std::string& directory, std::vector<std::string>* paths,
                     std::string* error) override;
  bool ReadFile(const std::string& path, SourceFile* file, std::string* error) override;
};

}

#endif
