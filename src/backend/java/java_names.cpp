#include "java_names.h"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "pinyin_table.h"

namespace tiecodec {

JavaNames::JavaNames(const SymbolTable& symbols, const BackendConfig& config)
    : symbols_(symbols), config_(config) {}

namespace {

bool IsPinyinMode(const BackendConfig& config) {
  return config.friendly_name == "pinyin";
}
bool IsRandomMode(const BackendConfig& config) {
  return config.friendly_name == "random";
}

bool ReadFileText(const std::string& path, std::string* text) {
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open())
    return false;
  std::ostringstream buffer;
  buffer << input.rdbuf();
  *text = buffer.str();
  return true;
}

std::vector<std::string> IdentifierTokens(const std::string& identifier) {
  std::vector<std::string> tokens;
  std::size_t index = 0;
  while (index < identifier.size()) {
    const unsigned char lead = static_cast<unsigned char>(identifier[index]);
    if (lead < 0x80) {
      if (std::isalnum(lead) != 0) {
        tokens.emplace_back(1, static_cast<char>(lead));
      }
      ++index;
      continue;
    }
    std::size_t length = 1;
    if ((lead & 0xE0u) == 0xC0u)
      length = 2;
    else if ((lead & 0xF0u) == 0xE0u)
      length = 3;
    else if ((lead & 0xF8u) == 0xF0u)
      length = 4;
    char32_t codepoint = 0;
    if (length == 2)
      codepoint = lead & 0x1Fu;
    else if (length == 3)
      codepoint = lead & 0x0Fu;
    else if (length == 4)
      codepoint = lead & 0x07u;
    ++index;
    for (std::size_t i = 1; i < length && index < identifier.size(); ++i, ++index) {
      codepoint = (codepoint << 6) | (static_cast<unsigned char>(identifier[index]) & 0x3Fu);
    }
    if (const char* pinyin = PinyinFor(codepoint)) {
      tokens.emplace_back(pinyin);
    } else {
      tokens.emplace_back(1, 'x');
    }
  }
  return tokens;
}

std::string CamelCase(const std::string& identifier) {
  std::string result;
  for (std::string token : IdentifierTokens(identifier)) {
    if (token.empty())
      continue;
    token[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(token[0])));
    result += token;
  }
  return result.empty() ? identifier : result;
}

std::string LowerCamelCase(const std::string& identifier) {
  std::string result;
  bool first_segment = true;
  std::size_t begin = 0;
  while (begin <= identifier.size()) {
    const std::size_t underscore = identifier.find('_', begin);
    const std::string segment = identifier.substr(
        begin, underscore == std::string::npos ? std::string::npos : underscore - begin);
    if (!first_segment)
      result.push_back('_');
    std::string segment_result;
    bool first_token = true;
    for (std::string token : IdentifierTokens(segment)) {
      if (token.empty())
        continue;
      if (first_token) {
        if (first_segment) {
          for (char& c : token) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
          }
        } else {
          token[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(token[0])));
        }
        first_token = false;
      } else {
        token[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(token[0])));
      }
      segment_result += token;
    }
    result += segment_result.empty() ? segment : segment_result;
    first_segment = false;
    if (underscore == std::string::npos)
      break;
    begin = underscore + 1;
  }
  return result.empty() ? identifier : result;
}

std::string PackageInitials(const std::string& segment) {
  std::string result;
  for (const std::string& token : IdentifierTokens(segment)) {
    if (token.empty())
      continue;
    result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(token[0]))));
  }
  return result.empty() ? segment : result;
}

std::string TransformPackage(const std::string& package) {
  std::string result;
  std::size_t begin = 0;
  while (begin <= package.size()) {
    const std::size_t dot = package.find('.', begin);
    const std::string segment =
        package.substr(begin, dot == std::string::npos ? std::string::npos : dot - begin);
    if (!result.empty())
      result.push_back('.');
    result += PackageInitials(segment);
    if (dot == std::string::npos)
      break;
    begin = dot + 1;
  }
  return result;
}

}  // namespace

std::string PinyinLowerCamel(const std::string& identifier) {
  return LowerCamelCase(identifier);
}

std::string PinyinCamel(const std::string& identifier) {
  return CamelCase(identifier);
}

std::string JavaNames::RandomName(const std::string& source) const {
  const auto stable = stable_names_.find(source);
  if (stable != stable_names_.end())
    return stable->second;
  const auto existing = random_names_.find(source);
  if (existing != random_names_.end())
    return existing->second;
  const std::string name = "rn_" + std::to_string(++random_counter_);
  random_names_[source] = name;
  return name;
}

bool JavaNames::LoadStableNames(const std::string& path) {
  std::string data;
  if (!ReadFileText(path, &data) || data.size() < 11 ||
      data.compare(0, 4, std::string("TNM\x01", 4)) != 0) {
    return false;
  }
  std::size_t pos = 4;
  const auto read_u32 = [&data, &pos](std::uint32_t* value) {
    if (pos + 4 > data.size())
      return false;
    std::uint32_t result = 0;
    for (int i = 0; i < 4; ++i) {
      result |= static_cast<std::uint32_t>(static_cast<unsigned char>(data[pos + i])) << (i * 8);
    }
    pos += 4;
    *value = result;
    return true;
  };
  const auto read_string = [&data, &pos](std::string* value) {
    std::uint32_t length = 0;
    if (pos + 4 > data.size())
      return false;
    for (int i = 0; i < 4; ++i) {
      length |= static_cast<std::uint32_t>(static_cast<unsigned char>(data[pos + i])) << (i * 8);
    }
    pos += 4;
    if (pos + length > data.size())
      return false;
    value->assign(data, pos, length);
    pos += length;
    return true;
  };
  std::uint32_t count = 0;
  if (!read_u32(&count))
    return false;
  stable_names_.clear();
  for (std::uint32_t i = 0; i < count; ++i) {
    std::string source;
    std::string output;
    if (!read_string(&source) || !read_string(&output))
      return false;
    stable_names_[source] = output;
  }
  return true;
}

bool JavaNames::SaveNames(const std::string& path) const {
  std::string data("TNM\x01", 4);
  const auto write_u32 = [&data](std::uint32_t value) {
    for (int i = 0; i < 4; ++i) {
      data.push_back(static_cast<char>((value >> (i * 8)) & 0xFF));
    }
  };
  const auto write_string = [&data, &write_u32](const std::string& value) {
    write_u32(static_cast<std::uint32_t>(value.size()));
    data += value;
  };
  write_u32(static_cast<std::uint32_t>(random_names_.size() + stable_names_.size()));
  std::map<std::string, std::string> merged = stable_names_;
  for (const auto& entry : random_names_)
    merged[entry.first] = entry.second;
  for (const auto& entry : merged) {
    write_string(entry.first);
    write_string(entry.second);
  }
  std::ofstream output(path, std::ios::binary);
  if (!output.is_open())
    return false;
  output.write(data.data(), static_cast<std::streamsize>(data.size()));
  return output.good();
}

std::string JavaNames::ClassSimpleName(const std::string& simple) const {
  if (IsRandomMode(config_))
    return RandomName(simple);
  return IsPinyinMode(config_) ? CamelCase(simple) : simple;
}

std::string JavaNames::MemberSimpleName(const std::string& name) const {
  if (IsRandomMode(config_))
    return RandomName(name);
  return IsPinyinMode(config_) ? LowerCamelCase(name) : name;
}

std::string JavaNames::PackageName(const std::string& package) const {
  if (IsRandomMode(config_))
    return RandomName(package);
  return IsPinyinMode(config_) ? TransformPackage(package) : package;
}

std::string JavaNames::RNamespace() const {
  return PackageName(config_.namespace_name);
}

std::string JavaNames::ClassName(const ClassInfo& klass) const {
  if (!klass.output_name.empty())
    return klass.output_name;
  const std::string& qualified = klass.qualified_name;
  const std::size_t dot = qualified.rfind('.');
  return ClassSimpleName(dot == std::string::npos ? qualified : qualified.substr(dot + 1));
}

std::string JavaNames::Package(const ClassInfo& klass) const {
  if (!klass.package_name.empty()) {
    const std::string mapped = symbols_.OutputPackageFor(klass.package_name);
    if (!mapped.empty())
      return PackageName(mapped);
  }
  if (!klass.output_package.empty())
    return PackageName(klass.output_package);
  if (!klass.package_name.empty())
    return PackageName(klass.package_name);
  return PackageName(config_.namespace_name);
}

namespace {

std::string OperatorName(const std::string& symbol) {
  static const std::map<std::string, std::string> names = {
      {"+", "operator_plus"},
      {"-", "operator_minus"},
      {"*", "operator_mul"},
      {"/", "operator_div"},
      {"%", "operator_mod"},
      {"==", "operator_equal"},
      {"!=", "operator_not_equal"},
      {"<", "operator_less"},
      {">", "operator_greater"},
      {"<=", "operator_less_equal"},
      {">=", "operator_greater_equal"},
      {"&", "operator_and"},
      {"|", "operator_or"},
      {"^", "operator_xor"},
      {"=", "operator_assign"},
      {"[]", "operator_index"},
      {"[]=", "operator_index_assign"},
      {"<=>", "operator_swap"},
      {"->", "operator_arrow"},
      {"&&", "operator_and"},
      {"||", "operator_or"},
  };
  const auto it = names.find(symbol);
  return it == names.end() ? std::string() : it->second;
}

}  // namespace

std::string JavaNames::LocalName(const std::string& name) const {
  if (IsRandomMode(config_))
    return RandomName(name);
  return IsPinyinMode(config_) ? LowerCamelCase(name) : name;
}

std::string JavaNames::MemberName(const Member& member) const {
  for (const Annotation& annotation : member.annotations) {
    if (annotation.name != "输出名")
      continue;
    std::string value = annotation.StringArgument("参数");
    if (value.empty() && !annotation.arguments.empty())
      value = annotation.arguments.front().value;
    if (!value.empty())
      return value;
  }
  if (IsRandomMode(config_))
    return RandomName(member.name);
  if (!IsPinyinMode(config_))
    return member.name;
  bool property_read = member.kind == MemberKind::kPropertyRead;
  bool property_write = member.kind == MemberKind::kPropertyWrite;
  bool constant = member.kind == MemberKind::kConstant;
  for (const Annotation& annotation : member.annotations) {
    if (annotation.name == "属性读")
      property_read = true;
    if (annotation.name == "属性写")
      property_write = true;
    if (annotation.name == "常量")
      constant = true;
  }
  if (property_read)
    return "get" + CamelCase(member.name);
  if (property_write)
    return "set" + CamelCase(member.name);
  if (constant) {
    std::string result;
    for (const std::string& token : IdentifierTokens(member.name)) {
      if (!result.empty())
        result.push_back('_');
      for (char c : token) {
        result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
      }
    }
    if (!result.empty())
      return result;
  }
  const std::string operator_name = OperatorName(member.name);
  if (!operator_name.empty())
    return operator_name;
  return LowerCamelCase(member.name);
}

std::string JavaNames::ImportPath(const ClassInfo& klass) const {
  const std::string package = Package(klass);
  return package.empty() ? ClassName(klass) : package + "." + ClassName(klass);
}

std::string JavaNames::BaseName(const ClassInfo& klass) const {
  if (klass.has_java_alias)
    return {};
  if (klass.declaration == nullptr || klass.declaration->base_name.empty())
    return {};
  const ClassInfo* base = symbols_.Resolve(klass.declaration->base_name, &klass);
  if (base == nullptr)
    return klass.declaration->base_name;
  if (base->has_java_alias)
    return base->java_alias;
  return ClassName(*base);
}

std::string JavaNames::GeneratedName(const std::string& hint, int index) const {
  if (IsRandomMode(config_))
    return "rn_" + std::to_string(++random_counter_);
  return hint + std::to_string(index);
}

}  // namespace tiecodec
