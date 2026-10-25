#include "android_resource_binder.h"

#include "android_effect_types.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "constants.h"
#include "file_search.h"
#include "java_type_mapper.h"
#include "output_sink.h"
#include "pinyin_table.h"
#include "text_util.h"

namespace tiecodec {
namespace {

std::string XmlEscape(const std::string& text) {
  std::string result;
  for (char c : text) {
    switch (c) {
      case '&':
        result += "&amp;";
        break;
      case '<':
        result += "&lt;";
        break;
      case '>':
        result += "&gt;";
        break;
      case '"':
        result += "&quot;";
        break;
      default:
        result.push_back(c);
        break;
    }
  }
  return result;
}

std::string StyleAlias(const std::string& name) {
  static const std::map<std::string, std::string> aliases = {
      {"主题色", "colorPrimary"},           {"强调色", "colorPrimaryDark"},
      {"状态栏颜色", "statusBarColor"},     {"导航栏颜色", "navigationBarColor"},
      {"窗口背景色", "windowBackground"},   {"组件前景色", "colorForeground"},
      {"组件背景色", "colorBackground"},    {"组件选中色", "colorAccent"},
      {"组件默认色", "colorControlNormal"}, {"按钮背景色", "colorButtonNormal"},
      {"文本主题色", "textColorPrimary"},
  };
  const auto it = aliases.find(name);
  return it == aliases.end() ? name : it->second;
}

std::string JoinPath(const std::string& base, const std::string& relative) {
  const std::filesystem::path path = std::filesystem::path(base) / relative;
  return path.lexically_normal().string();
}

std::string BaseName(const std::string& path) {
  return std::filesystem::path(path).filename().string();
}

std::string ExtensionOf(const std::string& path) {
  return std::filesystem::path(path).extension().string();
}

}

AndroidResourceBinder::AndroidResourceBinder(const SymbolTable& symbols, const TargetConfig& config,
                                             const SemanticModel& semantics,
                                             const EffectStore& effects)
    : symbols_(symbols),
      config_(config),
      effects_(effects),
      names_(symbols, config, semantics),
      types_(symbols, &names_) {}

std::string AndroidResourceBinder::RuleOf(const ClassInfo& klass) const {
  if (klass.declaration == nullptr)
    return {};
  const AndroidValueOutputRuleSpec* spec =
      effects_.Get<AndroidValueOutputRuleSpec>(klass.declaration);
  return spec == nullptr ? std::string() : spec->rule;
}

std::string AndroidResourceBinder::ConstantTypeOf(const ClassInfo& klass) const {
  if (klass.declaration == nullptr)
    return {};
  const AndroidConstantTypeSpec* spec = effects_.Get<AndroidConstantTypeSpec>(klass.declaration);
  return spec == nullptr ? std::string() : spec->type;
}

std::string AndroidResourceBinder::LiteralValue(const Member& member) const {
  if (member.expression_body == nullptr)
    return {};
  const Expr& expression = *member.expression_body;
  if (expression.kind == ExprKind::kLiteral) {
    if (!expression.raw.empty() && expression.raw.front() == '"')
      return expression.text;
    return expression.text;
  }
  return {};
}

std::string AndroidResourceBinder::OutputNameOf(const Member& member) const {
  return names_.MemberName(member);
}

bool AndroidResourceBinder::IsResourceField(const ClassInfo& owner, const Member& member) const {
  if (owner.declaration == nullptr)
    return false;
  if (member.type_name.empty())
    return false;
  const ClassInfo* type = symbols_.Resolve(member.type_name, &owner);
  if (type == nullptr || type->declaration == nullptr)
    return false;
  return !RuleOf(*type).empty();
}

std::string AndroidResourceBinder::FieldType(const ClassInfo& owner, const Member& member) const {
  const ClassInfo* type = symbols_.Resolve(member.type_name, &owner);
  if (type == nullptr)
    return "int";
  const std::string constant_type = ConstantTypeOf(*type);
  if (constant_type.empty())
    return types_.MapType(member.type_name, &owner);
  return types_.MapType(constant_type, &owner);
}

std::string AndroidResourceBinder::ResourceImport(const ClassInfo& owner) const {
  (void)owner;
  const std::string package = config_.Option(option::kRNamespace, config_.namespace_name);
  return package + ".R";
}

std::string AndroidResourceBinder::ResolvePath(const ClassInfo& owner,
                                               const std::string& path) const {
  if (path.empty())
    return {};
  const std::filesystem::path direct(path);
  if (direct.is_absolute())
    return path;
  std::vector<std::string> prefixes;
  for (const std::string& prefix : config_.ListOption(option::kSearchResPrefix))
    prefixes.push_back(prefix);
  for (const std::string& prefix : config_.ListOption(option::kSearchLibPrefix))
    prefixes.push_back(prefix);
  std::string source_file;
  if (owner.declaration != nullptr)
    source_file = owner.declaration->file;
  const std::filesystem::path resolved = file_search::ResolvePath(path, source_file, prefixes);
  if (!resolved.empty())
    return resolved.string();
  return JoinPath(std::filesystem::path(source_file).parent_path().string(), path);
}

void AndroidResourceBinder::RegisterOwnedName(const std::string& group,
                                              const std::string& identifier,
                                              const std::string& value_key, std::string* name) {
  auto& by_value = value_names_[group];
  auto& used = used_names_[group];
  const auto existing = by_value.find(value_key);
  if (existing != by_value.end()) {
    *name = existing->second;
    return;
  }
  std::string base = ResourceNameOf(identifier);
  std::string candidate = base;
  if (used.count(candidate) != 0) {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "_%08x", ResourceNameHash(identifier));
    candidate = base + buffer;
    int suffix = 1;
    while (used.count(candidate) != 0) {
      candidate = base + buffer + "_" + std::to_string(suffix++);
    }
  }
  used.insert(candidate);
  by_value[value_key] = candidate;
  *name = candidate;
}

bool AndroidResourceBinder::RegisterFileResource(const ClassInfo& owner, const Member& member,
                                                 const std::string& rule,
                                                 const std::string& value) {
  std::string group;
  std::string target_dir;
  std::string r_kind;
  if (rule == annotation::kRuleDrawable) {
    group = "drawable";
    target_dir = "res/drawable";
    r_kind = "drawable";
  } else if (rule == annotation::kRuleDrawableXxh) {
    group = "drawable";
    target_dir = "res/drawable-xxhdpi";
    r_kind = "drawable";
  } else if (rule == annotation::kRuleAnim) {
    group = "anim";
    target_dir = "res/anim";
    r_kind = "anim";
  } else if (rule == annotation::kRuleXml) {
    group = "xml";
    target_dir = "res/xml";
    r_kind = "xml";
  } else {
    return false;
  }
  resources_empty_ = false;
  has_res_ = true;
  if (!value.empty() && value.front() == '@') {
    pending_field_ = "R." + r_kind + "." + value.substr(1);
    return true;
  }
  const std::string resolved = ResolvePath(owner, value);
  std::string name;
  RegisterOwnedName(group, OutputNameOf(member), resolved, &name);
  const std::string target = target_dir + "/" + name + ExtensionOf(resolved);
  CopyFileTo(resolved, target);
  pending_field_ = "R." + r_kind + "." + name;
  return true;
}

bool AndroidResourceBinder::RegisterStringResource(const Member& member, const std::string& value) {
  resources_empty_ = false;
  has_strings_ = true;
  std::string name;
  RegisterOwnedName("string", OutputNameOf(member), OutputNameOf(member), &name);
  StringEntry entry;
  entry.name = name;
  entry.value = value;
  entry.sort_key = name;
  strings_[""].push_back(std::move(entry));
  if (const AndroidI18nSpec* i18n = effects_.Get<AndroidI18nSpec>(&member)) {
    for (const AndroidI18nEntry& entry : i18n->entries) {
      StringEntry localized;
      localized.name = OutputNameOf(member);
      localized.value = entry.text;
      localized.sort_key = ResourceNameOf(localized.name);
      strings_[entry.locale].push_back(std::move(localized));
    }
  }
  pending_field_ = "R.string." + name;
  return true;
}

bool AndroidResourceBinder::RegisterThemeResource(const ClassInfo& owner, const Member& member,
                                                  const std::string& value) {
  resources_empty_ = false;
  const AndroidThemeConfigSpec* config = effects_.Get<AndroidThemeConfigSpec>(&member);
  if (config == nullptr) {
    pending_field_ = "R.style." + value;
    (void)owner;
    return true;
  }
  has_styles_ = true;
  StyleEntry style;
  style.name = value;
  style.parent = config->parent;
  const auto items = ParseTable(config->table);
  for (auto it = items.rbegin(); it != items.rend(); ++it) {
    StyleItem item;
    item.name = StyleAlias(it->first);
    item.value = it->second;
    style.items.push_back(std::move(item));
  }
  bool found = false;
  for (StyleEntry& existing : styles_) {
    if (existing.name != style.name)
      continue;
    existing = style;
    found = true;
  }
  if (!found)
    styles_.push_back(std::move(style));
  pending_field_ = "R.style." + value;
  (void)owner;
  return true;
}

std::string AndroidResourceBinder::BindField(const ClassInfo& owner, const Member& member) {
  const ClassInfo* type = symbols_.Resolve(member.type_name, &owner);
  if (type == nullptr)
    return {};
  const std::string rule = RuleOf(*type);
  if (rule.empty())
    return {};
  const std::string value = LiteralValue(member);
  pending_field_.clear();
  if (rule == "@android/string") {
    if (RegisterStringResource(member, value))
      return pending_field_;
    return {};
  }
  if (rule == "@android/theme") {
    if (RegisterThemeResource(owner, member, value))
      return pending_field_;
    return {};
  }
  if (RegisterFileResource(owner, member, rule, value))
    return pending_field_;
  return {};
}

void AndroidResourceBinder::CopyFileTo(const std::string& source, const std::string& target) {
  const std::string prefixed = output_prefix_ + target;
  for (const FileCopy& copy : copies_) {
    if (copy.target == prefixed)
      return;
  }
  copies_.push_back(FileCopy{source, prefixed});
}

void AndroidResourceBinder::CopyDirectory(const std::string& directory,
                                          const std::string& target_prefix) {
  std::error_code error;
  if (!std::filesystem::exists(directory, error))
    return;
  if (std::filesystem::is_regular_file(directory, error)) {
    CopyFileTo(directory, target_prefix + BaseName(directory));
    return;
  }
  directories_.insert(target_prefix);
  for (std::filesystem::recursive_directory_iterator it(directory, error), end; it != end;
       it.increment(error)) {
    if (error)
      return;
    if (!it->is_regular_file(error))
      continue;
    const std::string relative = std::filesystem::relative(it->path(), directory, error).string();
    CopyFileTo(it->path().string(), target_prefix + relative);
  }
}

void AndroidResourceBinder::CopyResourceDirectory(const std::string& directory) {
  std::error_code error;
  if (!std::filesystem::exists(directory, error))
    return;
  if (std::filesystem::is_regular_file(directory, error)) {
    CopyFileTo(directory, "res/" + BaseName(directory));
    return;
  }
  static const std::set<std::string> kResDirs = {"drawable", "drawable-xxhdpi", "layout", "xml",
                                                 "anim",     "values",          "mipmap", "raw",
                                                 "menu",     "color",           "font"};
  for (std::filesystem::recursive_directory_iterator it(directory, error), end; it != end;
       it.increment(error)) {
    if (error)
      return;
    if (!it->is_regular_file(error))
      continue;
    const std::string relative = std::filesystem::relative(it->path(), directory, error).string();
    const std::size_t slash = relative.find('/');
    const std::string root = relative.substr(0, slash);
    if (slash == std::string::npos || kResDirs.count(root) == 0)
      continue;
    CopyFileTo(it->path().string(), "res/" + relative);
  }
}

void AndroidResourceBinder::ProcessClassAnnotations(const ClassInfo& klass) {
  if (klass.declaration == nullptr)
    return;
  if (const AndroidGlobalThemeSpec* theme =
          effects_.Get<AndroidGlobalThemeSpec>(klass.declaration)) {
    global_theme_ = theme->name;
  }
  const AndroidResourceCopies* copies = effects_.Get<AndroidResourceCopies>(klass.declaration);
  if (copies == nullptr)
    return;
  for (const AndroidResourceCopySpec& spec : copies->entries) {
    if (spec.path.empty())
      continue;
    const std::string resolved = ResolvePath(klass, spec.path);
    switch (spec.kind) {
      case AndroidResourceCopySpec::Kind::kExtraResource:
        CopyDirectory(resolved, "assets/");
        break;
      case AndroidResourceCopySpec::Kind::kImage:
        CopyFileTo(resolved, "res/drawable/" + BaseName(resolved));
        break;
      case AndroidResourceCopySpec::Kind::kAnimation:
        CopyFileTo(resolved, "res/anim/" + BaseName(resolved));
        break;
      case AndroidResourceCopySpec::Kind::kLayout:
        CopyFileTo(resolved, "res/layout/" + BaseName(resolved));
        break;
      case AndroidResourceCopySpec::Kind::kXml:
        CopyFileTo(resolved, "res/xml/" + BaseName(resolved));
        break;
      case AndroidResourceCopySpec::Kind::kAndroidResource:
        CopyResourceDirectory(resolved);
        break;
      case AndroidResourceCopySpec::Kind::kExternalJavaFile:
        CopyDirectory(resolved, source_prefix_);
        break;
      case AndroidResourceCopySpec::Kind::kNativeLibrary:
        CopyDirectory(resolved, "jniLibs/");
        break;
    }
  }
}

void AndroidResourceBinder::ProcessMemberManifest(const ClassInfo& klass, const Member& member) {
  (void)klass;
  const AndroidManifestSpecs* specs = effects_.Get<AndroidManifestSpecs>(&member);
  if (specs == nullptr)
    return;
  for (const AndroidManifestEntrySpec& spec : specs->entries) {
    std::string node = spec.node.empty() ? "application" : Unescape(UnwrapQuotes(spec.node));
    const std::string content = Unescape(UnwrapQuotes(spec.value));
    if (content.empty())
      continue;
    if (node == "application" || node == "default")
      application_children_.push_back(content);
  }
}

std::vector<std::pair<std::string, std::string>> AndroidResourceBinder::ParseTable(
    const std::string& text) {
  std::vector<std::pair<std::string, std::string>> items;
  std::string body = text;
  if (!body.empty() && body.front() == '{')
    body = body.substr(1);
  if (!body.empty() && body.back() == '}')
    body.pop_back();
  std::size_t begin = 0;
  int depth = 0;
  char quote = 0;
  std::vector<std::string> parts;
  for (std::size_t i = 0; i < body.size(); ++i) {
    const char c = body[i];
    if (quote != 0) {
      if (c == '\\')
        ++i;
      else if (c == quote)
        quote = 0;
      continue;
    }
    if (c == '"' || c == '\'') {
      quote = c;
      continue;
    }
    if (c == '{' || c == '[' || c == '(')
      ++depth;
    if (c == '}' || c == ']' || c == ')')
      --depth;
    if (c == ',' && depth == 0) {
      parts.push_back(body.substr(begin, i - begin));
      begin = i + 1;
    }
  }
  if (begin < body.size())
    parts.push_back(body.substr(begin));
  for (const std::string& part : parts) {
    const std::size_t equals = part.find('=');
    if (equals == std::string::npos)
      continue;
    std::string key = part.substr(0, equals);
    std::string value = part.substr(equals + 1);
    while (!key.empty() && (key.back() == ' ' || key.back() == '\t'))
      key.pop_back();
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
      value.erase(value.begin());
    }
    items.emplace_back(key, Unescape(UnwrapQuotes(value)));
  }
  return items;
}

void AndroidResourceBinder::Write(OutputSink& sink) const {
  const std::string header = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<resources>\n";
  std::map<std::string, std::vector<StringEntry>> all_strings = strings_;
  all_strings[""];
  for (const auto& locale_entries : all_strings) {
    const std::string& locale = locale_entries.first;
    std::vector<StringEntry> entries = locale_entries.second;
    std::stable_sort(
        entries.begin(), entries.end(),
        [](const StringEntry& a, const StringEntry& b) { return a.sort_key < b.sort_key; });
    std::string xml = header;
    if (locale.empty()) {
      xml += "\t<string name=\"app_name\">" + XmlEscape(config_.app_name) + "</string>\n";
    }
    for (const StringEntry& entry : entries) {
      xml += "\t<string name=\"" + XmlEscape(entry.name) + "\">" + XmlEscape(entry.value) +
             "</string>\n";
    }
    xml += "</resources>";
    const std::string path =
        output_prefix_ +
        (locale.empty() ? "res/values/strings.xml" : "res/values-" + locale + "/strings.xml");
    sink.Write(path, xml);
  }

  std::string styles =
      "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
      "<resources xmlns:tools=\"http://schemas.android.com/tools\">\n";
  if (global_theme_.empty()) {
    styles += "\t<style name=\"AppTheme\" parent=\"@android:style/Theme.Material.Light\">\n";
    styles += "\t</style>\n";
  }
  std::vector<StyleEntry> sorted = styles_;
  std::stable_sort(sorted.begin(), sorted.end(),
                   [](const StyleEntry& a, const StyleEntry& b) { return a.name < b.name; });
  for (StyleEntry& style : sorted) {
    styles += "\t<style name=\"" + XmlEscape(style.name) + "\" parent=\"" +
              XmlEscape(style.parent) + "\">\n";
    for (const StyleItem& item : style.items) {
      styles +=
          "\t\t<item name=\"" + XmlEscape(item.name) + "\">" + XmlEscape(item.value) + "</item>\n";
    }
    styles += "\t</style>\n";
  }
  styles += "</resources>";
  if (has_styles_ || global_theme_.empty()) {
    sink.Write(output_prefix_ + "res/values/styles.xml", styles);
  }

  for (const std::string& directory : directories_) {
    sink.CreateDirectory(output_prefix_ + directory);
  }
  for (const FileCopy& copy : copies_)
    sink.CopyFile(copy.source, copy.target);
}

}
