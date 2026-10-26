#include "cxx_names.h"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "constants.h"
#include "cxx_language_constants.h"
#include "options.h"
#include "pinyin_table.h"
#include "text_util.h"

namespace tiecodec {

std::vector<std::string> SplitCxxNamespace(const std::string& package) {
  std::vector<std::string> segments;
  if (package.empty())
    return segments;
  for (const std::string& raw : SplitDelimited(package, '.')) {
    const std::string segment = Trim(raw);
    if (!segment.empty())
      segments.push_back(segment);
  }
  return segments;
}

std::string CxxHeaderGuardOf(const std::string& ns_path, const std::string& class_name,
                             bool reference_shape) {
  if (reference_shape)
    return ns_path + "_" + class_name + "_H";
  const std::string key = ns_path + "." + class_name;
  std::uint64_t hash = 1469598103934665603ULL;
  std::string ascii;
  for (unsigned char c : key) {
    hash ^= c;
    hash *= 1099511628211ULL;
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
      ascii.push_back(static_cast<char>(std::toupper(c)));
  }
  if (ascii.size() > 32)
    ascii.resize(32);
  char suffix[17];
  std::snprintf(suffix, sizeof(suffix), "%016llx", static_cast<unsigned long long>(hash));
  return "TIECODEC_GEN_" + ascii + "_" + suffix + "_H";
}

std::string CxxOperatorMethodName(const std::string& symbol) {
  static const std::map<std::string, std::string> names = {
      {"+", "operator_add"},
      {"-", "operator_subtract"},
      {"*", "operator_multiply"},
      {"/", "operator_divide"},
      {"%", "operator_modulo"},
      {"==", "operator_equal"},
      {"!=", "operator_not_equal"},
      {"<", "operator_less"},
      {">", "operator_greater"},
      {"<=", "operator_less_equal"},
      {">=", "operator_greater_equal"},
      {"&", "operator_bit_and"},
      {"|", "operator_bit_or"},
      {"^", "operator_bit_xor"},
      {"?", "operator_question"},
      {kOperatorAssign, "operator_assign"},
      {kOperatorIndex, "operator_array_access"},
      {kOperatorIndexAssign, "operator_array_assign"},
      {kOperatorSwap, "operator_swap"},
      {"->", "operator_arrow"},
      {"&&", "operator_logical_and"},
      {"||", "operator_logical_or"},
  };
  const auto it = names.find(symbol);
  return it == names.end() ? std::string() : it->second;
}

CxxNames::CxxNames(const SymbolTable& symbols, const TargetConfig& config,
                   const SemanticModel& semantics)
    : symbols_(symbols), config_(config), semantics_(semantics) {}

std::vector<std::string> CxxNames::NamespaceSegments(const ClassInfo& klass) const {
  std::vector<std::string> segments;
  std::string package;
  if (!klass.package_name.empty()) {
    const std::string mapped = symbols_.OutputPackageFor(klass.package_name);
    package = mapped.empty() ? klass.package_name : mapped;
  }
  if (package.empty())
    package = klass.output_package;
  if (package.empty())
    package = klass.package_name;
  if (package.empty())
    package = config_.namespace_name;
  if (PinyinMode() && !package.empty()) {
    std::string joined;
    for (const std::string& segment : SplitCxxNamespace(package)) {
      if (!joined.empty())
        joined.push_back('_');
      joined += PackageInitials(segment);
    }
    return {joined};
  }
  if (RandomMode() && !package.empty() && package != kCxxRootNamespace)
    return {RandomName(package)};
  return SplitCxxNamespace(package);
}

bool CxxNames::ReferenceShape() const {
  return config_.Flag(option::kDisableFtcFeatures);
}

std::string CxxNames::NamespacePath(const ClassInfo& klass) const {
  const std::vector<std::string> segments = NamespaceSegments(klass);
  if (segments.empty())
    return std::string();
  std::vector<std::string> out;
  for (const std::string& segment : segments) {
    if (segment == kCxxRootNamespace)
      continue;
    out.push_back(segment);
  }
  if (out.empty())
    return kCxxRootNamespace;
  return JoinStrings(out, ReferenceShape() ? "." : kCxxNamespaceSeparator);
}

std::string CxxNames::ClassName(const ClassInfo& klass) const {
  if (!klass.output_name.empty())
    return klass.output_name;
  const std::string& qualified = klass.qualified_name;
  const std::size_t dot = qualified.rfind('.');
  const std::string simple = dot == std::string::npos ? qualified : qualified.substr(dot + 1);
  if (RandomMode())
    return RandomName(simple);
  if (PinyinMode())
    return CamelCase(simple);
  return simple;
}

bool CxxNames::PinyinMode() const {
  return IsPinyinNaming(config_.friendly_name);
}

bool CxxNames::RandomMode() const {
  return IsRandomNaming(config_.friendly_name);
}

std::string CxxNames::RandomName(const std::string& source) const {
  const auto found = random_names_.find(source);
  if (found != random_names_.end())
    return found->second;
  const std::string generated = "rn_" + std::to_string(++random_counter_);
  random_names_.emplace(source, generated);
  return generated;
}

std::string CxxNames::PinyinMemberName(const Member& member) const {
  if (member.kind == MemberKind::kConstant) {
    std::string result;
    for (const std::string& token : IdentifierTokens(member.name)) {
      if (token.empty())
        continue;
      if (!result.empty())
        result.push_back('_');
      for (char c : token)
        result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    return result.empty() ? member.name : result;
  }
  return LowerCamelCase(member.name);
}

std::string CxxNames::HeaderGuard(const ClassInfo& klass) const {
  return CxxHeaderGuardOf(NamespacePath(klass), ClassName(klass), ReferenceShape());
}

std::string CxxNames::Qualify(const ClassInfo& klass) const {
  const std::string ns = NamespacePath(klass);
  if (ns.empty())
    return ClassName(klass);
  return ns + kCxxNamespaceSeparator + ClassName(klass);
}

std::string CxxNames::Package(const ClassInfo& klass) const {
  return NamespacePath(klass);
}

std::string CxxNames::ImportPath(const ClassInfo& klass) const {
  return Qualify(klass);
}

std::string CxxNames::BaseName(const ClassInfo& klass) const {
  if (klass.declaration == nullptr || klass.declaration->base_name.empty())
    return {};
  if (!klass.external_base.empty())
    return klass.external_base;
  const ClassInfo* base = symbols_.Resolve(klass.declaration->base_name, &klass);
  if (base == nullptr)
    return klass.declaration->base_name;
  if (base->HasExternalType(kCxxTypeAliasKey))
    return base->ExternalType(kCxxTypeAliasKey);
  if (NamespacePath(*base) == NamespacePath(klass))
    return ClassName(*base);
  return std::string("::") + Qualify(*base);
}

std::string CxxNames::MemberName(const Member& member) const {
  const MemberFacts* facts = semantics_.FindMember(&member);
  if (facts != nullptr && !facts->output_name.empty())
    return facts->output_name;
  if (member.name == kCxxToStringSourceName)
    return kCxxToStringMethod;
  const unsigned char first = member.name.empty() ? 0 : static_cast<unsigned char>(member.name[0]);
  const bool may_be_operator = first != 0 && first < 0x80 &&
                               !((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') ||
                                 (first >= '0' && first <= '9') || first == '_');
  if (may_be_operator) {
    const std::string operator_name = CxxOperatorMethodName(member.name);
    if (!operator_name.empty())
      return operator_name;
  }
  if (RandomMode())
    return RandomName(member.name);
  if (PinyinMode())
    return PinyinMemberName(member);
  return member.name;
}

std::string CxxNames::LocalName(const std::string& name) const {
  return name;
}

std::string CxxNames::GeneratedName(const std::string& hint, int index) const {
  return hint + std::to_string(index);
}

}
