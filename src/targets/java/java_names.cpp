#include "java_names.h"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "binary_io.h"
#include "constants.h"
#include "pinyin_table.h"

namespace tiecodec {

JavaNames::JavaNames(const SymbolTable& symbols, const TargetConfig& config,
                     const SemanticModel& semantics)
    : symbols_(symbols), config_(config), semantics_(semantics) {}

namespace {

constexpr char kNamesMagic[4] = {'T', 'N', 'M', 1};
constexpr std::size_t kNamesMagicSize = sizeof(kNamesMagic);
constexpr std::size_t kMinimumNamesFileSize = kNamesMagicSize + binary::kU32Size;

bool IsPinyinMode(const TargetConfig& config) {
  return IsPinyinNaming(config.friendly_name);
}
bool IsRandomMode(const TargetConfig& config) {
  return IsRandomNaming(config.friendly_name);
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

}

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
  const std::string name = std::string(kRandomNamePrefix) + std::to_string(++random_counter_);
  random_names_[source] = name;
  return name;
}

bool JavaNames::LoadStableNames(const std::string& path) {
  std::string data;
  if (!ReadFileText(path, &data) || data.size() < kMinimumNamesFileSize ||
      data.compare(0, kNamesMagicSize, std::string(kNamesMagic, kNamesMagicSize)) != 0) {
    return false;
  }
  std::size_t position = kNamesMagicSize;
  std::uint32_t count = 0;
  if (!binary::ReadU32(data, &position, &count))
    return false;
  stable_names_.clear();
  for (std::uint32_t i = 0; i < count; ++i) {
    std::string source;
    std::string output;
    if (!binary::ReadString(data, &position, &source) ||
        !binary::ReadString(data, &position, &output)) {
      return false;
    }
    stable_names_[source] = output;
  }
  return true;
}

bool JavaNames::SaveNames(const std::string& path) const {
  std::string data(kNamesMagic, kNamesMagicSize);
  const std::uint32_t total =
      static_cast<std::uint32_t>(random_names_.size() + stable_names_.size());
  binary::AppendU32(&data, total);
  std::map<std::string, std::string> merged = stable_names_;
  for (const auto& entry : random_names_)
    merged[entry.first] = entry.second;
  for (const auto& entry : merged) {
    binary::AppendString(&data, entry.first);
    binary::AppendString(&data, entry.second);
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
      {"?", "operator_question"},
      {kOperatorAssign, "operator_assign"},
      {kOperatorIndex, "operator_array_access"},
      {kOperatorIndexAssign, "operator_array_assign"},
      {kOperatorSwap, "operator_swap"},
      {"->", "operator_arrow"},
      {"&&", "operator_and"},
      {"||", "operator_or"},
  };
  const auto it = names.find(symbol);
  return it == names.end() ? std::string() : it->second;
}

}

std::string JavaNames::LocalName(const std::string& name) const {
  if (IsRandomMode(config_))
    return RandomName(name);
  return IsPinyinMode(config_) ? LowerCamelCase(name) : name;
}

std::string JavaNames::MemberName(const Member& member) const {
  const MemberFacts* facts = semantics_.FindMember(&member);
  if (facts != nullptr && !facts->output_name.empty())
    return facts->output_name;
  if (IsRandomMode(config_))
    return RandomName(member.name);
  const unsigned char first = member.name.empty() ? 0 : static_cast<unsigned char>(member.name[0]);
  const bool may_be_operator = first != 0 && first < 0x80 &&
                               !((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') ||
                                 (first >= '0' && first <= '9') || first == '_' || first == '$');
  if (may_be_operator) {
    const std::string operator_name = OperatorName(member.name);
    if (!operator_name.empty())
      return operator_name;
  }
  if (!IsPinyinMode(config_))
    return member.name;
  const bool property_read = member.kind == MemberKind::kPropertyRead;
  const bool property_write = member.kind == MemberKind::kPropertyWrite;
  const bool constant = member.kind == MemberKind::kConstant;
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
  return LowerCamelCase(member.name);
}

std::string JavaNames::AsyncContextName(const std::string& source_class_name,
                                        const std::string& member_name) const {
  if (IsRandomMode(config_))
    return GeneratedName(kRandomNamePrefix, 0);
  if (IsPinyinMode(config_)) {
    return SegmentPascal(source_class_name) + "_" + CamelCase(member_name) +
           SegmentPascal(kAsyncContextSuffix);
  }
  return source_class_name + "_" + member_name + kAsyncContextSuffix;
}

std::string JavaNames::AsyncMethodName(const Member& member) const {
  if (IsRandomMode(config_))
    return GeneratedName(kRandomNamePrefix, 0);
  if (IsPinyinMode(config_))
    return LowerCamelCase(member.name + kAsyncMethodSuffix);
  return MemberName(member) + kAsyncMethodSuffix;
}

std::string JavaNames::ImportPath(const ClassInfo& klass) const {
  const std::string package = Package(klass);
  return package.empty() ? ClassName(klass) : package + "." + ClassName(klass);
}

std::string JavaNames::BaseName(const ClassInfo& klass) const {
  if (klass.HasExternalType("java"))
    return {};
  if (klass.declaration == nullptr || klass.declaration->base_name.empty())
    return {};
  const ClassInfo* base = symbols_.Resolve(klass.declaration->base_name, &klass);
  if (base == nullptr)
    return klass.declaration->base_name;
  if (base->HasExternalType("java"))
    return base->ExternalType("java");
  return ClassName(*base);
}

std::string JavaNames::GeneratedName(const std::string& hint, int index) const {
  if (IsRandomMode(config_))
    return std::string(kRandomNamePrefix) + std::to_string(++random_counter_);
  return hint + std::to_string(index);
}

}
