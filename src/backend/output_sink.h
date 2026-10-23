#ifndef TIECODEC_BACKEND_OUTPUT_SINK_H
#define TIECODEC_BACKEND_OUTPUT_SINK_H

#include <string>

namespace tiecodec {

class OutputSink {
public:
  virtual ~OutputSink() = default;

  virtual bool Write(const std::string& relative_path, const std::string& content) = 0;

  /// Copy an external file into the output tree (resources, icons).
  virtual bool CopyFile(const std::string& source_path, const std::string& relative_path) {
    (void)source_path;
    (void)relative_path;
    return false;
  }

  /// Ensure an empty directory exists in the output tree.
  virtual void CreateDirectory(const std::string& relative_path) {
    (void)relative_path;
  }
};

class FileSystemOutputSink : public OutputSink {
public:
  explicit FileSystemOutputSink(std::string root) : root_(std::move(root)) {}
  bool Write(const std::string& relative_path, const std::string& content) override;
  bool CopyFile(const std::string& source_path, const std::string& relative_path) override;
  void CreateDirectory(const std::string& relative_path) override;

private:
  std::string root_;
};

}  // namespace tiecodec

#endif
