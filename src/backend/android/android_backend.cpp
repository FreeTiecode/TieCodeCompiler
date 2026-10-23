#include "android_backend.h"

#include <functional>

#include "java_type_mapper.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>

#include "internal/helpers.h"
#include "symbol_resolution.h"
#include "usage_analysis.h"

namespace tiecodec {

using namespace detail;

AndroidBackend::AndroidBackend(const SymbolTable& symbols) : symbols_(symbols) {}

namespace {

bool ResolveConstantString(const SymbolTable& symbols, const ClassInfo& context,
                           const std::string& reference, std::string* value) {
  const std::size_t dot = reference.rfind('.');
  if (dot == std::string::npos)
    return false;
  const ClassInfo* owner = symbols.Resolve(reference.substr(0, dot), &context);
  if (owner == nullptr || owner->declaration == nullptr)
    return false;
  const ClassInfo* member_owner = nullptr;
  const Member* member =
      FindMemberInChain(symbols, *owner, reference.substr(dot + 1), &member_owner);
  if (member == nullptr || member->expression_body == nullptr)
    return false;
  if (member->expression_body->kind != ExprKind::kLiteral)
    return false;
  const Expr& literal = *member->expression_body;
  if (!literal.raw.empty() && (literal.raw[0] == '"' || literal.raw[0] == '\'')) {
    *value = literal.text;
    return true;
  }
  return false;
}

}  // namespace

bool AndroidBackend::Generate(const ClassInfo* entry, const BackendConfig& config,
                              Diagnostics& diagnostics) {
  options_ = config;
  diagnostics_ = &diagnostics;
  names_ = std::make_unique<JavaNames>(symbols_, options_);
  const auto stable_names_path = options_.options.find("stable-names");
  if (stable_names_path != options_.options.end() && !stable_names_path->second.empty()) {
    names_->LoadStableNames(stable_names_path->second);
  }
  options_.options["r-namespace"] = names_->RNamespace();
  types_ = std::make_unique<JavaTypeMapper>(symbols_, names_.get());
  resources_ = std::make_unique<AndroidResourceBinder>(symbols_, options_);
  resources_->set_output_prefix(MainPrefix());
  resources_->set_source_prefix(SourceDir());
  emitter_ = std::make_unique<JavaEmitter>(symbols_, options_, *names_, *types_, diagnostics,
                                           resources_.get());
  sink_ = std::make_unique<FileSystemOutputSink>(config.output_dir);

  emitter_->RunAnalysisLints();
  ValidateEnvironment(contract_, symbols_, diagnostics_);
  if (diagnostics_->has_errors())
    return false;
  if (entry == nullptr) {
    diagnostics_->Fatal("当前环境中找不到入口类 \"" + std::string(kEntryClassName) + "\"",
                        "output.entry.class-not-found");
    return false;
  }
  return GenerateInternal(entry);
}

bool AndroidBackend::WriteFile(const std::string& relative_path, const std::string& content) {
  if (sink_ != nullptr && sink_->Write(relative_path, content))
    return true;
  if (diagnostics_ != nullptr) {
    diagnostics_->Fatal("无法写入输出文件: " + options_.output_dir + "/" + relative_path,
                        "io.write");
  }
  return false;
}

void AndroidBackend::PrepareLibraries() {
  library_entries_.clear();
  std::set<std::string> seen;
  const std::vector<const ClassInfo*>& emitted =
      output_classes_ != nullptr ? *output_classes_ : std::vector<const ClassInfo*>();
  const bool hard_mode =
      options_.options.count("hard-mode") != 0 && options_.options.at("hard-mode") == "1";
  const std::string libs_dir = GradleMode() ? "app/libs" : "libs";
  std::vector<std::filesystem::path> lib_search_prefixes;
  {
    const auto prefixes = options_.options.find("search-lib-prefix");
    if (prefixes != options_.options.end()) {
      std::string list = prefixes->second;
      std::size_t begin = 0;
      while (begin <= list.size()) {
        const std::size_t end = list.find(';', begin);
        const std::string prefix =
            list.substr(begin, end == std::string::npos ? std::string::npos : end - begin);
        if (!prefix.empty())
          lib_search_prefixes.emplace_back(prefix);
        if (end == std::string::npos)
          break;
        begin = end + 1;
      }
    }
  }
  const auto resolve_library_path = [&](const ClassDecl& declaration, const std::string& path) {
    std::filesystem::path candidate(path);
    if (!candidate.is_absolute()) {
      std::vector<std::filesystem::path> bases;
      bases.push_back(std::filesystem::path(declaration.file).parent_path());
      bases.push_back(std::filesystem::path("."));
      for (const std::filesystem::path& prefix : lib_search_prefixes)
        bases.push_back(prefix);
      std::error_code base_error;
      for (const std::filesystem::path& base : bases) {
        const std::filesystem::path full = base / candidate;
        if (std::filesystem::exists(full, base_error)) {
          candidate = full;
          break;
        }
      }
    }
    std::error_code canonical_error;
    const std::filesystem::path canonical =
        std::filesystem::weakly_canonical(candidate, canonical_error);
    return (canonical_error ? candidate.lexically_normal() : canonical).string();
  };
  for (const ClassInfo* klass : emitted) {
    if (klass == nullptr || klass->declaration == nullptr)
      continue;
    const ClassDecl& declaration = *klass->declaration;
    for (const Annotation& annotation : declaration.annotations) {
      if (annotation.name != "外部依赖库")
        continue;
      std::string path = annotation.StringArgument("参数");
      if (path.empty() && !annotation.arguments.empty()) {
        path = annotation.arguments.front().value;
      }
      if (path.empty())
        continue;
      bool compile_only = false;
      for (const AnnotationArgument& argument : annotation.arguments) {
        if (argument.name == "不打包")
          compile_only = argument.value == "真";
      }
      std::string file = resolve_library_path(declaration, path);
      bool copied = false;
      if (hard_mode && !compile_only) {
        const std::string base = std::filesystem::path(file).filename().string();
        const std::string relative = libs_dir + "/" + base;
        if (sink_ != nullptr)
          sink_->CopyFile(file, relative);
        copied = true;
        std::error_code abs_error;
        const std::filesystem::path absolute_root =
            std::filesystem::absolute(options_.output_dir, abs_error);
        file = (abs_error ? std::filesystem::path(options_.output_dir) : absolute_root).string() +
               "/" + relative;
      }
      if (!seen.insert(file).second)
        continue;
      LibraryEntry entry;
      entry.file = file;
      entry.compile_only = compile_only;
      entry.copied = copied;
      library_entries_.push_back(std::move(entry));
    }
  }
  const bool hard_mode_enabled =
      options_.options.count("hard-mode") != 0 && options_.options.at("hard-mode") == "1";
  if (!library_entries_.empty() && sink_ != nullptr && hard_mode_enabled) {
    sink_->CreateDirectory(libs_dir);
  }
}

std::string AndroidBackend::LibsJson() const {
  if (library_entries_.empty())
    return "[]";
  std::string json = "[\n";
  for (std::size_t i = 0; i < library_entries_.size(); ++i) {
    json += "    {\n";
    json += std::string("      \"compileOnly\": ") +
            (library_entries_[i].compile_only ? "true" : "false") + ",\n";
    json += "      \"file\": \"" + library_entries_[i].file + "\"\n";
    json += "    }";
    if (i + 1 != library_entries_.size())
      json += ",";
    json += "\n";
  }
  json += "  ]";
  return json;
}

bool AndroidBackend::GradleMode() const {
  const auto it = options_.options.find("android.gradle");
  return it != options_.options.end() && it->second == "1";
}

std::string AndroidBackend::MainPrefix() const {
  return GradleMode() ? "app/src/main/" : "";
}

std::string AndroidBackend::SourceDir() const {
  return GradleMode() ? "java/" : "src/";
}

std::string AndroidBackend::ReadTemplate(const std::string& relative) const {
  std::ifstream input(std::string(TIECODEC_TEMPLATE_DIR) + "/" + relative, std::ios::binary);
  if (!input.is_open())
    return {};
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

std::string AndroidBackend::AppBuildGradle() const {
  std::string text = ReadTemplate("gradle/app/build.gradle.tmpl");
  const auto replace_all = [](std::string* target, const std::string& from, const std::string& to) {
    if (from.empty())
      return;
    std::size_t position = 0;
    while ((position = target->find(from, position)) != std::string::npos) {
      target->replace(position, from.size(), to);
      position += to.size();
    }
  };
  replace_all(&text, "namespace '结绳.中文'", "namespace '" + names_->RNamespace() + "'");
  replace_all(&text, "applicationId \"js.zw\"", "applicationId \"" + options_.package_name + "\"");
  replace_all(&text, "minSdk 21", "minSdk " + std::to_string(options_.min_sdk));
  replace_all(&text, "targetSdk 28", "targetSdk " + std::to_string(options_.target_sdk));
  replace_all(&text, "versionCode 1", "versionCode " + std::to_string(options_.version_code));
  replace_all(&text, "versionName \"1.0\"", "versionName \"" + options_.version_name + "\"");
  const std::size_t deps = text.find("dependencies {");
  const std::size_t test_line =
      deps == std::string::npos ? std::string::npos : text.find("\n    testImplementation", deps);
  if (deps != std::string::npos && test_line != std::string::npos) {
    std::string body = "dependencies {\n";
    if (library_entries_.empty()) {
      body += "    implementation fileTree(dir: 'libs', include: ['*.aar', '*.jar'])\n";
    } else {
      body += "    \n";
      for (const LibraryEntry& entry : library_entries_) {
        if (entry.compile_only)
          body += "\tcompileOnly files('" + entry.file + "')\n";
      }
      for (const LibraryEntry& entry : library_entries_) {
        if (entry.compile_only)
          continue;
        if (entry.copied && GradleMode()) {
          body += "\timplementation files('libs/" +
                  std::filesystem::path(entry.file).filename().string() + "')\n";
        } else {
          body += "\timplementation files('" + entry.file + "')\n";
        }
      }
    }
    text = text.substr(0, deps) + body + text.substr(test_line);
  }
  return text;
}

bool AndroidBackend::WriteGradleProject() {
  const auto write_template = [this](const std::string& template_path,
                                     const std::string& output_path,
                                     const std::string& override_content) {
    const std::string content =
        override_content.empty() ? ReadTemplate(template_path) : override_content;
    if (content.empty())
      return true;
    return WriteFile(output_path, content);
  };
  std::string settings = ReadTemplate("gradle/settings.gradle");
  const std::string root_pattern = "rootProject.name = \"MyApp\"";
  const std::size_t root_pos = settings.find(root_pattern);
  if (root_pos != std::string::npos) {
    settings.replace(root_pos, root_pattern.size(),
                     "rootProject.name = \"" + options_.app_name + "\"");
  }
  if (!write_template("gradle/settings.gradle", "settings.gradle", settings))
    return false;
  if (!write_template("gradle/build.gradle", "build.gradle", std::string()))
    return false;
  if (!write_template("gradle/gradle.properties", "gradle.properties", std::string()))
    return false;
  if (!write_template("gradle/gradlew", "gradlew", std::string()))
    return false;
  if (!write_template("gradle/gradlew.bat", "gradlew.bat", std::string()))
    return false;
  if (!write_template("gradle/gradle/wrapper/gradle-wrapper.properties",
                      "gradle/wrapper/gradle-wrapper.properties", std::string()))
    return false;
  if (!write_template("gradle/app/proguard-rules.pro", "app/proguard-rules.pro", std::string()))
    return false;
  if (!write_template("gradle/app/build.gradle.tmpl", "app/build.gradle", AppBuildGradle()))
    return false;
  if (sink_ != nullptr) {
    sink_->CopyFile(
        std::string(TIECODEC_TEMPLATE_DIR) + "/gradle/gradle/wrapper/gradle-wrapper.jar",
        "gradle/wrapper/gradle-wrapper.jar");
  }
  return true;
}

std::uint64_t AndroidBackend::SourceFileIndex(const std::string& file) {
  const auto it = source_file_index_.find(file);
  if (it != source_file_index_.end())
    return it->second;
  const std::uint64_t index = static_cast<std::uint64_t>(source_mapping_.source_files.size());
  source_mapping_.source_files.push_back(file);
  source_file_index_[file] = index;
  return index;
}

void AndroidBackend::RecordLineMap(const ClassInfo& klass, const ClassOutput& output) {
  JavaFileMapping file;
  file.name = names_->ClassName(klass) + ".java";
  for (std::size_t i = 0; i < output.line_refs.size(); ++i) {
    if (i == 0)
      continue;
    const LineRef& ref = output.line_refs[i];
    if (ref.line <= 0 || ref.file.empty())
      continue;
    JavaLineEntry entry;
    entry.java_line = static_cast<std::uint64_t>(i + 1);
    entry.source_index = SourceFileIndex(ref.file);
    entry.source_line = static_cast<std::uint64_t>(ref.line);
    file.entries.push_back(entry);
  }
  source_mapping_.java_files.push_back(std::move(file));
}

void AndroidBackend::CollectMappingNames() {
  source_mapping_.names.clear();
  std::set<std::string> seen;
  const auto add_pair = [this, &seen](const std::string& source_name,
                                      const std::string& output_name) {
    if (source_name.empty() || output_name.empty())
      return;
    const std::string key = source_name + "\n" + output_name;
    if (!seen.insert(key).second)
      return;
    source_mapping_.names.emplace_back(source_name, output_name);
  };
  if (output_classes_ == nullptr)
    return;
  for (const ClassInfo* klass : *output_classes_) {
    if (klass == nullptr || klass->declaration == nullptr)
      continue;
    if (!klass->package_name.empty())
      add_pair(klass->package_name, klass->package_name);
    const std::string simple = klass->qualified_name.substr(klass->qualified_name.rfind('.') + 1);
    add_pair(simple, names_->ClassName(*klass));
    for (const Member& member : klass->declaration->members) {
      if (used_members_ != nullptr && used_members_->count(&member) == 0)
        continue;
      std::string output_name = names_->MemberName(member);
      if (member.kind == MemberKind::kEventBlock && !member.event_object.empty() &&
          symbols_.Resolve(member.event_object, klass) == nullptr) {
        output_name = member.event_object + "$" + output_name;
      }
      add_pair(member.name, output_name);
    }
  }
}

namespace {

std::string UnquoteAnnotationText(const std::string& value) {
  std::string text = value;
  if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
    text = text.substr(1, text.size() - 2);
  }
  std::string unescaped;
  unescaped.reserve(text.size());
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '\\' && i + 1 < text.size()) {
      unescaped.push_back(text[++i]);
    } else {
      unescaped.push_back(text[i]);
    }
  }
  return unescaped;
}

bool LiteralArgumentText(const Expr* expression, std::string* value) {
  if (expression == nullptr || expression->kind != ExprKind::kLiteral)
    return false;
  if (!expression->raw.empty() && (expression->raw[0] == '"' || expression->raw[0] == '\'')) {
    *value = UnquoteAnnotationText(expression->raw);
    return true;
  }
  if (expression->text == "真") {
    *value = "true";
    return true;
  }
  if (expression->text == "假") {
    *value = "false";
    return true;
  }
  *value = expression->text;
  return !value->empty();
}

const Member* ResolveCallTarget(const SymbolTable& symbols, const ClassInfo& klass,
                                const Expr& callee) {
  if (callee.kind == ExprKind::kIdentifier) {
    const ClassInfo* owner = nullptr;
    if (const Member* member = FindMemberInChain(symbols, klass, callee.text, &owner)) {
      return member;
    }
    return nullptr;
  }
  if (callee.kind == ExprKind::kFieldAccess && !callee.children.empty() &&
      callee.children[0] != nullptr) {
    const Expr& receiver = *callee.children[0];
    const ClassInfo* receiver_class = nullptr;
    if (receiver.kind == ExprKind::kIdentifier) {
      receiver_class = symbols.Resolve(receiver.text, &klass);
      if (receiver_class == nullptr) {
        if (const Member* receiver_member =
                FindMemberInChain(symbols, klass, receiver.text, nullptr)) {
          receiver_class = symbols.Resolve(receiver_member->type_name, &klass);
        }
      }
    }
    if (receiver_class == nullptr)
      return nullptr;
    const ClassInfo* owner = nullptr;
    return FindMemberInChain(symbols, *receiver_class, callee.text, &owner);
  }
  return nullptr;
}

std::string SubstituteManifestParameters(const std::string& content, const Member& member,
                                         const Expr& call) {
  std::string result = content;
  for (std::size_t i = 0; i < member.parameters.size(); ++i) {
    const std::string placeholder = "${" + member.parameters[i].name + "}";
    if (result.find(placeholder) == std::string::npos)
      continue;
    const Expr* argument = (i + 1 < call.children.size()) ? call.children[i + 1].get() : nullptr;
    std::string value;
    if (!LiteralArgumentText(argument, &value))
      continue;
    std::size_t position = 0;
    while ((position = result.find(placeholder, position)) != std::string::npos) {
      result.replace(position, placeholder.size(), value);
      position += value.size();
    }
  }
  return result;
}

}  // namespace

bool AndroidBackend::WriteProjectFiles(const ClassInfo& entry) {
  const std::string package = options_.package_name;
  const std::string activity = names_->Package(entry) + "." + names_->ClassName(entry);

  const auto strip_quotes = [](std::string value) {
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
      value = value.substr(1, value.size() - 2);
    }
    return value;
  };

  const auto collect_call_site_fragments = [&](const ClassInfo& klass,
                                               std::vector<std::string>* properties,
                                               std::vector<std::string>* children) {
    if (used_members_ == nullptr)
      return;
    std::function<void(const Expr*)> walk_expression = [&](const Expr* expression) {
      if (expression == nullptr)
        return;
      for (const ExprPtr& child : expression->children)
        walk_expression(child.get());
      if (expression->kind != ExprKind::kCall || expression->children.empty() ||
          expression->children[0] == nullptr) {
        return;
      }
      const Member* target = ResolveCallTarget(symbols_, klass, *expression->children[0]);
      if (target == nullptr)
        return;
      for (const Annotation& annotation : target->annotations) {
        if (annotation.name != "附加清单")
          continue;
        std::string node = "component.property";
        std::string content;
        for (const AnnotationArgument& argument : annotation.arguments) {
          if (argument.name == "节点")
            node = UnquoteAnnotationText(argument.value);
          else if (argument.name.empty() || argument.name == "参数") {
            content = UnquoteAnnotationText(argument.value);
          }
        }
        if (content.empty())
          continue;
        const std::string substituted = SubstituteManifestParameters(content, *target, *expression);
        if (node == "component.child") {
          children->push_back(substituted);
        } else if (node == "component.property") {
          properties->push_back(substituted);
        }
      }
    };
    std::function<void(const std::vector<StmtPtr>&)> walk_statements =
        [&](const std::vector<StmtPtr>& statements) {
          for (const StmtPtr& statement : statements) {
            if (statement == nullptr)
              continue;
            walk_expression(statement->expr.get());
            walk_expression(statement->loop_begin.get());
            walk_expression(statement->loop_end.get());
            walk_expression(statement->loop_step.get());
            for (const SwitchCase& switch_case : statement->cases) {
              for (const ExprPtr& value : switch_case.values)
                walk_expression(value.get());
              walk_statements(switch_case.body);
            }
            walk_statements(statement->body);
            walk_statements(statement->else_body);
          }
        };
    if (klass.declaration == nullptr)
      return;
    for (const Member& member : klass.declaration->members) {
      if (used_members_->count(&member) == 0)
        continue;
      walk_statements(member.body);
    }
  };

  std::set<const ClassInfo*> output_class_set(output_classes_->begin(), output_classes_->end());
  std::vector<const ClassInfo*> ordered_classes;
  for (const std::string& class_name : symbols_.class_names()) {
    const ClassInfo* candidate = symbols_.Find(class_name);
    if (candidate == nullptr || output_class_set.count(candidate) == 0)
      continue;
    ordered_classes.push_back(candidate);
  }

  struct ComponentEntry {
    std::string kind;
    std::string qualified;
    std::string source_qualified;
    std::string theme;
    bool is_entry = false;
    std::vector<std::string> properties;
    std::vector<std::string> children;
    std::vector<std::string> call_properties;
    std::vector<std::string> call_children;
  };

  std::vector<ComponentEntry> components;
  for (const ClassInfo* klass : ordered_classes) {
    if (klass == nullptr || klass->declaration == nullptr)
      continue;
    if (klass->has_java_alias || klass->forbid_new)
      continue;
    if (klass->declaration->name == "窗口")
      continue;

    std::string kind;
    const ClassInfo* cursor = klass;
    std::set<const ClassInfo*> visited;
    while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
      const std::string simple =
          cursor->qualified_name.substr(cursor->qualified_name.rfind('.') + 1);
      if (simple == "服务")
        kind = "service";
      else if (simple == "广播接收器")
        kind = "receiver";
      else if (simple == "窗口")
        kind = kind.empty() ? "activity" : kind;
      if (!kind.empty() && simple == kind)
        break;
      if (kind.empty() && cursor->has_java_alias &&
          cursor->java_alias.find("Activity") != std::string::npos) {
        kind = "activity";
      }
      if (!kind.empty())
        break;
      if (cursor->declaration->base_name.empty())
        break;
      cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
    }
    const bool is_entry = klass->declaration->name == kEntryClassName;
    if (kind.empty() && !is_entry)
      continue;

    ComponentEntry component;
    component.kind = kind.empty() ? "activity" : kind;
    component.is_entry = is_entry;
    component.qualified = names_->Package(*klass) + "." + names_->ClassName(*klass);
    component.source_qualified = klass->qualified_name;
    for (const Annotation& annotation : klass->declaration->annotations) {
      std::string value;
      if (annotation.name == "窗口主题") {
        value = annotation.StringArgument("参数");
        if (value.empty() && !annotation.arguments.empty()) {
          value = annotation.arguments.front().value;
        }
        component.theme = strip_quotes(value);
        continue;
      }
      if (annotation.name != "附加清单")
        continue;
      std::string node = "component.property";
      for (const AnnotationArgument& argument : annotation.arguments) {
        if (argument.name == "节点")
          node = strip_quotes(argument.value);
        else if (argument.name.empty() || argument.name == "参数") {
          value = strip_quotes(argument.value);
        }
      }
      if (value.empty())
        continue;
      if (node == "component.child")
        component.children.push_back(value);
      else if (node == "component.property")
        component.properties.push_back(value);
    }
    collect_call_site_fragments(*klass, &component.call_properties, &component.call_children);
    components.push_back(std::move(component));
  }

  // Components are emitted in sorted name order; the entry activity goes first
  // when it owns fields of other emitted components, otherwise last.
  std::sort(components.begin(), components.end(),
            [](const ComponentEntry& a, const ComponentEntry& b) {
              if (a.is_entry != b.is_entry)
                return false;
              return a.source_qualified < b.source_qualified;
            });
  auto entry_position = std::find_if(components.begin(), components.end(),
                                     [](const ComponentEntry& entry) { return entry.is_entry; });
  if (entry_position != components.end()) {
    bool entry_references_component = false;
    for (const Member& member : entry.declaration->members) {
      const ClassInfo* member_class = symbols_.Resolve(member.type_name, &entry);
      if (member_class == nullptr || member_class->declaration == nullptr)
        continue;
      const std::string member_qualified =
          names_->Package(*member_class) + "." + names_->ClassName(*member_class);
      if (member_qualified == names_->Package(entry) + "." + names_->ClassName(entry)) {
        continue;
      }
      for (const ComponentEntry& component : components) {
        if (!component.is_entry && component.qualified == member_qualified) {
          entry_references_component = true;
        }
      }
    }
    ComponentEntry entry_component = *entry_position;
    components.erase(entry_position);
    if (entry_references_component) {
      components.insert(components.begin(), entry_component);
    } else {
      components.push_back(entry_component);
    }
  }

  std::string manifest;
  manifest += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
  manifest += "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"";
  if (!GradleMode()) {
    manifest += "\n\tpackage=\"" + package + "\"";
  }
  manifest += ">\n";

  std::vector<std::string> permissions;
  const auto add_permissions = [&](const std::vector<Annotation>& annotations) {
    for (const Annotation& annotation : annotations) {
      if (annotation.name != "附加权限")
        continue;
      for (const AnnotationArgument& argument : annotation.arguments) {
        std::string permission;
        if (!argument.value.empty() && argument.value.front() == '"') {
          permission = UnquoteAnnotationText(argument.value);
        } else if (!ResolveConstantString(symbols_, entry, argument.value, &permission)) {
          permission = UnquoteAnnotationText(argument.value);
        }
        if (permission.empty())
          continue;
        if (std::find(permissions.begin(), permissions.end(), permission) == permissions.end()) {
          permissions.push_back(permission);
        }
      }
    }
  };
  for (const ClassInfo* klass : *output_classes_) {
    if (klass == nullptr || klass->declaration == nullptr)
      continue;
    add_permissions(klass->declaration->annotations);
    for (const Member& member : klass->declaration->members) {
      if (used_members_ != nullptr && used_members_->count(&member) == 0)
        continue;
      add_permissions(member.annotations);
    }
  }
  for (const std::string& permission : permissions) {
    manifest += "\t<uses-permission android:name=\"" + permission + "\"/>\n";
  }
  manifest += "\t<application\n";
  if (!options_.app_icon.empty()) {
    manifest += "\t\tandroid:icon=\"@drawable/icon\"\n";
    if (sink_ != nullptr) {
      sink_->CopyFile(options_.app_icon, MainPrefix() + "res/drawable/icon.png");
    }
  }
  manifest += "\t\tandroid:requestLegacyExternalStorage=\"true\"\n";
  manifest += "\t\tandroid:usesCleartextTraffic=\"true\"\n";
  manifest += "\t\tandroid:supportsRtl=\"true\"\n";
  manifest += "\t\tandroid:label=\"@string/app_name\"\n";
  const auto release_option = options_.options.find("release");
  const bool release_mode =
      release_option != options_.options.end() && release_option->second != "0";
  if (!release_mode) {
    manifest += "\t\tandroid:debuggable=\"true\"\n";
  }
  const std::string app_theme =
      resources_->global_theme().empty() ? "AppTheme" : resources_->global_theme();
  manifest += "\t\tandroid:theme=\"@style/" + app_theme + "\">\n";
  for (const ComponentEntry& component : components) {
    manifest += "\t\t<" + component.kind + "\n";
    for (const std::string& property : component.call_properties) {
      manifest += "\t\t\t" + property + "\n";
    }
    if (component.is_entry) {
      manifest += "\t\t\tandroid:exported=\"true\"\n";
      manifest += "\t\t\tandroid:label=\"@string/app_name\"\n";
    }
    for (const std::string& property : component.properties) {
      manifest += "\t\t\t" + property + "\n";
    }
    if (!component.theme.empty()) {
      manifest += "\t\t\tandroid:theme=\"@style/" + component.theme + "\"\n";
    }
    manifest += "\t\t\tandroid:name=\"" + component.qualified + "\">\n";
    if (component.is_entry) {
      manifest += "\t\t\t<intent-filter>\n";
      manifest += "\t\t\t\t<action android:name=\"android.intent.action.MAIN\" />\n";
      manifest += "\t\t\t\t<category android:name=\"android.intent.category.LAUNCHER\" />\n";
      manifest += "\t\t\t</intent-filter>\n";
    }
    for (const std::string& child : component.children) {
      manifest += "\t\t\t" + child + "\n";
    }
    for (const std::string& child : component.call_children) {
      manifest += "\t\t\t" + child + "\n";
    }
    manifest += "\t\t</" + component.kind + ">\n";
  }
  for (const std::string& fragment : resources_->application_children()) {
    manifest += "\t\t" + fragment + "\n";
  }
  manifest += "\t</application>\n";
  manifest += "</manifest>";
  if (!WriteFile(MainPrefix() + "AndroidManifest.xml", manifest))
    return false;

  resources_->Write(*sink_);

  PrepareLibraries();
  const std::string& out = options_.output_dir;
  const std::string prefix = MainPrefix();
  const std::string libs_dir = GradleMode() ? "libs" : "libs";
  std::string metadata;
  metadata += "{\n";
  metadata += "  \"appConfig\": {\n";
  metadata += "    \"appIcon\": \"" + options_.app_icon + "\",\n";
  metadata += "    \"appName\": \"" + options_.app_name + "\",\n";
  metadata += "    \"minSdk\": " + std::to_string(options_.min_sdk) + ",\n";
  metadata += "    \"targetSdk\": " + std::to_string(options_.target_sdk) + ",\n";
  metadata += "    \"versionCode\": " + std::to_string(options_.version_code) + ",\n";
  metadata += "    \"versionName\": \"" + options_.version_name + "\"\n";
  metadata += "  },\n";
  metadata += "  \"libs\": " + LibsJson() + ",\n";
  metadata += "  \"platform\": \"Android\",\n";
  metadata += "  \"productInfo\": {\n";
  metadata += "    \"assetsDir\": \"" + out + "/" + prefix + "assets\",\n";
  metadata += "    \"extraResDirs\": [],\n";
  metadata += "    \"jniLibsDir\": \"" + out + "/" + prefix + "jniLibs\",\n";
  metadata += "    \"libsDir\": \"" + out + "/" + (GradleMode() ? "app/" : "") + libs_dir + "\",\n";
  metadata += "    \"manifest\": \"" + out + "/" + prefix + "AndroidManifest.xml\",\n";
  metadata += "    \"packageName\": \"" + package + "\",\n";
  metadata += "    \"rNamespace\": \"" + names_->RNamespace() + "\",\n";
  metadata += "    \"resDir\": \"" + out + "/" + prefix + "res\",\n";
  const std::string source_dir = SourceDir();
  metadata += "    \"srcDir\": \"" + out + "/" + prefix +
              source_dir.substr(0, source_dir.size() - 1) + "\"\n";
  metadata += "  }\n";
  metadata += "}";
  if (!WriteFile("metadata.json", metadata))
    return false;

  const auto line_map_option = options_.options.find("line-map");
  if (line_map_option != options_.options.end() && !line_map_option->second.empty()) {
    CollectMappingNames();
    std::string mapping_error;
    if (!WriteSourceMapping(line_map_option->second, source_mapping_, &mapping_error)) {
      if (diagnostics_ != nullptr)
        diagnostics_->Fatal(mapping_error, "io.write");
      return false;
    }
  }
  const auto emit_names_path = options_.options.find("emit-names");
  if (emit_names_path != options_.options.end() && !emit_names_path->second.empty()) {
    names_->SaveNames(emit_names_path->second);
  }
  (void)entry;
  if (GradleMode() && !WriteGradleProject())
    return false;
  return true;
}

bool AndroidBackend::GenerateInternal(const ClassInfo* entry) {
  const OutputPlan plan = ComputeOutputPlan(symbols_, contract_, entry);
  const std::vector<const ClassInfo*>& output_classes = plan.classes;
  const std::set<const Member*>& used_members = plan.members;
  used_members_ = &used_members;
  output_classes_ = &output_classes;
  emitter_->SetPlan(used_members, plan.virtual_overrides, output_classes);
  std::set<const ClassInfo*> emitted_classes;
  for (const ClassInfo* klass : output_classes) {
    if (klass->forbid_output)
      continue;
    emitted_classes.insert(klass);
    const ClassOutput output = emitter_->EmitClass(*klass);
    if (!WriteFile(MainPrefix() + SourceDir() + names_->ClassName(*klass) + ".java", output.text)) {
      return false;
    }
    RecordLineMap(*klass, output);
    for (const auto& auxiliary : emitter_->TakeAuxiliaryClasses()) {
      if (!WriteFile(MainPrefix() + SourceDir() + auxiliary.first + ".java", auxiliary.second)) {
        return false;
      }
    }
    resources_->ProcessClassAnnotations(*klass);
    if (klass->declaration == nullptr)
      continue;
    for (const Member& member : klass->declaration->members) {
      if (used_members.count(&member) == 0)
        continue;
      if (member.kind == MemberKind::kVariable || member.kind == MemberKind::kConstant)
        continue;
      resources_->ProcessMemberManifest(*klass, member);
    }
    for (const Member& member : klass->declaration->members) {
      if (used_members.count(&member) == 0)
        continue;
      if (member.kind != MemberKind::kVariable && member.kind != MemberKind::kConstant)
        continue;
      resources_->ProcessMemberManifest(*klass, member);
    }
  }
  const auto emit_extra_class = [&](const ClassInfo* klass) -> bool {
    if (klass == nullptr || klass->declaration == nullptr || klass->forbid_output)
      return true;
    emitted_classes.insert(klass);
    const ClassOutput output = emitter_->EmitClass(*klass);
    if (!WriteFile(MainPrefix() + SourceDir() + names_->ClassName(*klass) + ".java", output.text)) {
      return false;
    }
    for (const auto& auxiliary : emitter_->TakeAuxiliaryClasses()) {
      if (!WriteFile(MainPrefix() + SourceDir() + auxiliary.first + ".java", auxiliary.second)) {
        return false;
      }
    }
    return true;
  };
  bool generic_progress = true;
  int generic_loop_guard = 0;
  while (generic_progress) {
    if (++generic_loop_guard > 500)
      break;
    generic_progress = false;
    const std::vector<JavaEmitter::GenericInstantiation> generics =
        emitter_->TakePendingGenericInstantiations();
    for (const JavaEmitter::GenericInstantiation& instantiation : generics) {
      const std::string text = emitter_->EmitGenericInstantiation(instantiation);
      if (!WriteFile(MainPrefix() + SourceDir() + instantiation.output_name + ".java", text)) {
        return false;
      }
      generic_progress = true;
    }
    const std::vector<const ClassInfo*> needed = emitter_->TakeGenericNeededClasses();
    for (const ClassInfo* klass : needed) {
      if (klass == nullptr || klass->declaration == nullptr)
        continue;
      if (!emitted_classes.insert(klass).second)
        continue;
      if (!emit_extra_class(klass))
        return false;
      const ClassInfo* cursor = klass;
      std::set<const ClassInfo*> visited;
      while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second &&
             !cursor->declaration->base_name.empty()) {
        cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
        if (cursor == nullptr)
          break;
        if (!emitted_classes.insert(cursor).second)
          continue;
        if (!emit_extra_class(cursor))
          return false;
      }
      generic_progress = true;
    }
  }
  const bool written = WriteProjectFiles(*entry);
  used_members_ = nullptr;
  output_classes_ = nullptr;
  return written;
}

}  // namespace tiecodec
