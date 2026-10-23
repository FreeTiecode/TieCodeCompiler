#include "android_resource_binder.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "java_type_mapper.h"
#include "output_sink.h"
#include "pinyin_table.h"

namespace tiecodec {
namespace {

std::string TrimQuotes(std::string text) {
  if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
    return text.substr(1, text.size() - 2);
  }
  return text;
}

std::string Unescape(std::string text) {
  std::string result;
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] != '\\' || i + 1 >= text.size()) {
      result.push_back(text[i]);
      continue;
    }
    const char next = text[++i];
    switch (next) {
      case 'n':
        result.push_back('\n');
        break;
      case 't':
        result.push_back('\t');
        break;
      case 'r':
        result.push_back('\r');
        break;
      case '"':
        result.push_back('"');
        break;
      case '\\':
        result.push_back('\\');
        break;
      default:
        result.push_back(next);
        break;
    }
  }
  return result;
}

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

const char* AnnotationValue(const std::vector<Annotation>& annotations, const std::string& name,
                            std::string* value) {
  for (const Annotation& annotation : annotations) {
    if (annotation.name != name)
      continue;
    std::string text = annotation.StringArgument("参数");
    if (text.empty() && !annotation.arguments.empty()) {
      text = annotation.arguments.front().value;
    }
    *value = Unescape(TrimQuotes(text));
    return value->c_str();
  }
  return nullptr;
}

const char* ArgumentOf(const Annotation& annotation, const std::string& name) {
  for (const AnnotationArgument& argument : annotation.arguments) {
    if (argument.name == name)
      return argument.value.c_str();
  }
  return nullptr;
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

bool HasAnnotation(const std::vector<Annotation>& annotations, const std::string& name) {
  for (const Annotation& annotation : annotations) {
    if (annotation.name == name)
      return true;
  }
  return false;
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

}  // namespace

AndroidResourceBinder::AndroidResourceBinder(const SymbolTable& symbols,
                                             const BackendConfig& config)
    : symbols_(symbols), config_(config), names_(symbols, config), types_(symbols, &names_) {}

std::string AndroidResourceBinder::RuleOf(const ClassInfo& klass) const {
  if (klass.declaration == nullptr)
    return {};
  for (const Annotation& annotation : klass.declaration->annotations) {
    if (annotation.name != "值输出规则")
      continue;
    std::string value = annotation.StringArgument("参数");
    if (value.empty() && !annotation.arguments.empty()) {
      value = annotation.arguments.front().value;
    }
    return Unescape(TrimQuotes(value));
  }
  return {};
}

std::string AndroidResourceBinder::ConstantTypeOf(const ClassInfo& klass) const {
  for (const Annotation& annotation : klass.declaration->annotations) {
    if (annotation.name != "常量类型")
      continue;
    std::string value =
        annotation.arguments.empty() ? std::string() : annotation.arguments.front().value;
    value = TrimQuotes(value);
    if (!value.empty())
      return value;
  }
  return {};
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
  for (const Annotation& annotation : member.annotations) {
    if (annotation.name != "输出名")
      continue;
    std::string value = annotation.StringArgument("参数");
    if (value.empty() && !annotation.arguments.empty()) {
      value = annotation.arguments.front().value;
    }
    return TrimQuotes(value);
  }
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
  const auto it = config_.options.find("r-namespace");
  const std::string package = it == config_.options.end() ? config_.namespace_name : it->second;
  return package + ".R";
}

std::string AndroidResourceBinder::ResolvePath(const ClassInfo& owner,
                                               const std::string& path) const {
  if (path.empty())
    return {};
  const std::filesystem::path direct(path);
  if (direct.is_absolute())
    return path;
  std::string base;
  if (owner.declaration != nullptr) {
    base = std::filesystem::path(owner.declaration->file).parent_path().string();
  }
  std::string candidate = JoinPath(base, path);
  if (std::filesystem::exists(candidate))
    return candidate;
  if (std::filesystem::exists(path))
    return path;
  const auto prefixes = config_.options.find("search-res-prefix");
  if (prefixes != config_.options.end()) {
    std::string list = prefixes->second;
    std::size_t begin = 0;
    while (begin <= list.size()) {
      const std::size_t end = list.find(';', begin);
      const std::string prefix =
          list.substr(begin, end == std::string::npos ? std::string::npos : end - begin);
      if (!prefix.empty()) {
        candidate = JoinPath(prefix, path);
        if (std::filesystem::exists(candidate))
          return candidate;
      }
      if (end == std::string::npos)
        break;
      begin = end + 1;
    }
  }
  return candidate;
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
  if (rule == "@android/drawable") {
    group = "drawable";
    target_dir = "res/drawable";
    r_kind = "drawable";
  } else if (rule == "@android/drawable-xxh") {
    group = "drawable";
    target_dir = "res/drawable-xxhdpi";
    r_kind = "drawable";
  } else if (rule == "@android/anim") {
    group = "anim";
    target_dir = "res/anim";
    r_kind = "anim";
  } else if (rule == "@android/xml") {
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
  for (const Annotation& annotation : member.annotations) {
    if (annotation.name != "国际化")
      continue;
    std::string locale;
    std::string text = annotation.StringArgument("参数");
    if (text.empty() && !annotation.arguments.empty())
      text = annotation.arguments.front().value;
    for (const AnnotationArgument& argument : annotation.arguments) {
      if (argument.name == "地区")
        locale = Unescape(TrimQuotes(argument.value));
      else if (argument.name.empty())
        text = argument.value;
    }
    StringEntry localized;
    localized.name = OutputNameOf(member);
    localized.value = Unescape(TrimQuotes(text));
    localized.sort_key = ResourceNameOf(localized.name);
    strings_[locale].push_back(std::move(localized));
  }
  pending_field_ = "R.string." + name;
  return true;
}

bool AndroidResourceBinder::RegisterThemeResource(const ClassInfo& owner, const Member& member,
                                                  const std::string& value) {
  resources_empty_ = false;
  const Annotation* config = nullptr;
  for (const Annotation& annotation : member.annotations) {
    if (annotation.name == "主题配置")
      config = &annotation;
  }
  if (config == nullptr) {
    pending_field_ = "R.style." + value;
    (void)owner;
    return true;
  }
  has_styles_ = true;
  StyleEntry style;
  style.name = value;
  for (const Annotation& annotation : member.annotations) {
    if (annotation.name != "主题配置")
      continue;
    std::string parent;
    std::string table;
    for (const AnnotationArgument& argument : annotation.arguments) {
      if (argument.name == "类型")
        parent = Unescape(TrimQuotes(argument.value));
      else if (argument.name.empty())
        table = argument.value;
    }
    style.parent = parent;
    const auto items = ParseTable(table);
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
      StyleItem item;
      item.name = StyleAlias(it->first);
      item.value = it->second;
      style.items.push_back(std::move(item));
    }
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
  for (const Annotation& annotation : klass.declaration->annotations) {
    if (annotation.name == "全局主题") {
      std::string value = annotation.StringArgument("参数");
      if (value.empty() && !annotation.arguments.empty()) {
        value = annotation.arguments.front().value;
      }
      global_theme_ = Unescape(TrimQuotes(value));
      continue;
    }
    std::string path = annotation.StringArgument("参数");
    if (path.empty() && !annotation.arguments.empty())
      path = annotation.arguments.front().value;
    path = Unescape(TrimQuotes(path));
    if (path.empty())
      continue;
    const std::string resolved = ResolvePath(klass, path);
    if (annotation.name == "附加资源") {
      CopyDirectory(resolved, "assets/");
    } else if (annotation.name == "安卓资源.图片") {
      CopyFileTo(resolved, "res/drawable/" + BaseName(resolved));
    } else if (annotation.name == "安卓资源.动画") {
      CopyFileTo(resolved, "res/anim/" + BaseName(resolved));
    } else if (annotation.name == "安卓资源.布局") {
      CopyFileTo(resolved, "res/layout/" + BaseName(resolved));
    } else if (annotation.name == "安卓资源.XML") {
      CopyFileTo(resolved, "res/xml/" + BaseName(resolved));
    } else if (annotation.name == "安卓资源") {
      CopyResourceDirectory(resolved);
    } else if (annotation.name == "外部Java文件") {
      CopyDirectory(resolved, source_prefix_);
    } else if (annotation.name == "外部动态库") {
      CopyDirectory(resolved, "jniLibs/");
    }
  }
}

void AndroidResourceBinder::ProcessMemberManifest(const ClassInfo& klass, const Member& member) {
  (void)klass;
  for (const Annotation& annotation : member.annotations) {
    if (annotation.name != "附加清单")
      continue;
    std::string node = "application";
    std::string content;
    for (const AnnotationArgument& argument : annotation.arguments) {
      if (argument.name == "节点")
        node = Unescape(TrimQuotes(argument.value));
      else if (argument.name.empty())
        content = Unescape(TrimQuotes(argument.value));
    }
    if (content.empty())
      continue;
    if (node == "application" || node == "default") {
      application_children_.push_back(content);
    }
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
    items.emplace_back(key, Unescape(TrimQuotes(value)));
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

}  // namespace tiecodec
