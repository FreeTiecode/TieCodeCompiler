#ifndef TIECODEC_BACKEND_OUTPUT_SINK_H
#define TIECODEC_BACKEND_OUTPUT_SINK_H

#include <filesystem>
#include <string>
#include <utility>

namespace tiecodec {

class OutputSink;
class Diagnostics;

bool WriteOutputFile(OutputSink& sink, const std::string& relative_path, const std::string& content,
                     const std::string& output_dir, Diagnostics* diagnostics);

class OutputSink {
public:
  virtual ~OutputSink() = default;

  virtual bool Write(const std::string& relative_path, const std::string& content) = 0;

  virtual bool CopyFile(const std::string& source_path, const std::string& relative_path) {
    (void)source_path;
    (void)relative_path;
    return false;
  }

  virtual void CreateDirectory(const std::string& relative_path) {
    (void)relative_path;
  }
};

class FileSystemOutputSink : public OutputSink {
public:
  explicit FileSystemOutputSink(std::string root);
  bool Write(const std::string& relative_path, const std::string& content) override;
  bool CopyFile(const std::string& source_path, const std::string& relative_path) override;
  void CreateDirectory(const std::string& relative_path) override;

private:
  std::string root_;
  std::filesystem::path canonical_root_;
};

}

#endif
