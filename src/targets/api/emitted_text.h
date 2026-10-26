#ifndef TIECODEC_TARGETS_API_EMITTED_TEXT_H
#define TIECODEC_TARGETS_API_EMITTED_TEXT_H

#include <cstring>
#include <string>
#include <vector>

#include "target.h"

namespace tiecodec {

inline constexpr int kClassMemberIndentLevel = 1;
inline constexpr int kMethodBodyIndentLevel = 2;
inline constexpr int kAsyncStateBodyIndentLevel = 3;
inline constexpr int kAnonymousClassBodyOffset = 2;

class EmittedText : public std::string {
public:
  EmittedText& operator+=(const std::string& value) {
    Add(value);
    return *this;
  }
  EmittedText& operator+=(const char* value) {
    if (value != nullptr)
      Add(value, std::char_traits<char>::length(value));
    return *this;
  }
  EmittedText& operator+=(char value) {
    std::string::operator+=(value);
    if (value == '\n')
      line_refs_.push_back(current_);
    return *this;
  }
  void append(const std::string& value) {
    Add(value);
  }
  void append(const char* value) {
    if (value != nullptr)
      Add(value, std::char_traits<char>::length(value));
  }
  void Mark(const std::string& file, int line) {
    current_.file = file;
    current_.line = line;
  }
  const std::vector<LineRef>& line_refs() const {
    return line_refs_;
  }
  void Finish() {
    if (!empty() && back() != '\n')
      line_refs_.push_back(current_);
  }
  void AppendText(const EmittedText& other) {
    Add(other);
    const std::vector<LineRef>& refs = other.line_refs();
    const std::size_t base = line_refs_.size() - refs.size();
    for (std::size_t i = 0; i < refs.size(); ++i) {
      if (refs[i].line > 0 && refs[i].file.size() > 0)
        line_refs_[base + i] = refs[i];
    }
  }

private:
  void Add(const std::string& value) {
    Add(value.data(), value.size());
  }
  void Add(const char* data, std::size_t size) {
    std::size_t start = 0;
    while (start < size) {
      const char* newline = static_cast<const char*>(std::memchr(data + start, '\n', size - start));
      if (newline == nullptr) {
        std::string::append(data + start, size - start);
        return;
      }
      const std::size_t end = static_cast<std::size_t>(newline - data) + 1;
      std::string::append(data + start, end - start);
      line_refs_.push_back(current_);
      start = end;
    }
  }
  std::vector<LineRef> line_refs_;
  LineRef current_;
};

struct ClassOutput {
  std::string text;
  std::vector<LineRef> line_refs;
};

struct AuxiliaryClass {
  std::string name;
  std::string text;
  std::vector<LineRef> line_refs;
  bool is_header = false;
  std::string directory;
  std::string extension;
};

}

#endif
