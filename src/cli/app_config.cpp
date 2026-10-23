#include "app_config.h"

#include <fstream>
#include <sstream>
#include <string>

namespace tiecodec {
namespace {

std::string ReadAll(const std::string& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open())
    return {};
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

bool FindString(const std::string& text, const std::string& key, std::string* value) {
  const std::string quoted = "\"" + key + "\"";
  std::size_t position = text.find(quoted);
  if (position == std::string::npos)
    return false;
  position = text.find(':', position + quoted.size());
  if (position == std::string::npos)
    return false;
  ++position;
  while (position < text.size() && (text[position] == ' ' || text[position] == '\t'))
    ++position;
  if (position >= text.size() || text[position] != '"')
    return false;
  ++position;
  while (position < text.size() && text[position] != '"') {
    if (text[position] == '\\' && position + 1 < text.size())
      ++position;
    value->push_back(text[position++]);
  }
  return true;
}

bool FindNumber(const std::string& text, const std::string& key, int* value) {
  const std::string quoted = "\"" + key + "\"";
  std::size_t position = text.find(quoted);
  if (position == std::string::npos)
    return false;
  position = text.find(':', position + quoted.size());
  if (position == std::string::npos)
    return false;
  ++position;
  while (position < text.size() && (text[position] == ' ' || text[position] == '\t'))
    ++position;
  const std::size_t begin = position;
  while (position < text.size() && text[position] >= '0' && text[position] <= '9')
    ++position;
  if (begin == position)
    return false;
  *value = std::stoi(text.substr(begin, position - begin));
  return true;
}

}  // namespace

bool LoadAppConfig(const std::string& path, BackendConfig* config, std::string* error) {
  const std::string text = ReadAll(path);
  if (text.empty()) {
    if (error != nullptr)
      *error = "无法读取 App 配置文件: " + path;
    return false;
  }
  std::string string_value;
  if (FindString(text, "appName", &string_value))
    config->app_name = string_value;
  string_value.clear();
  if (FindString(text, "appIcon", &string_value))
    config->app_icon = string_value;
  string_value.clear();
  if (FindString(text, "versionName", &string_value))
    config->version_name = string_value;
  int number = 0;
  if (FindNumber(text, "minSdk", &number))
    config->min_sdk = number;
  if (FindNumber(text, "targetSdk", &number))
    config->target_sdk = number;
  if (FindNumber(text, "versionCode", &number))
    config->version_code = number;
  return true;
}

}  // namespace tiecodec
