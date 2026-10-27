#include "android_platform_backend.h"

#include <functional>

#include "android_constants.h"
#include "android_effect_types.h"
#include "constants.h"
#include "file_search.h"
#include "java_language_constants.h"
#include "json_util.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <utility>

#include "internal/helpers.h"
#include "symbol_resolution.h"

namespace tiecodec {

using namespace detail;

AndroidPlatformBackend::AndroidPlatformBackend(const GenerationInput& input,
                                               std::shared_ptr<AndroidJavaExtension> extension)
    : symbols_(input.symbols),
      effects_(input.effects),
      options_(input.config),
      extension_(std::move(extension)),
      resources_(static_cast<AndroidResourceBinder*>(extension_->Resources())) {}

namespace {

constexpr const char* kAndroidManifestFile = "AndroidManifest.xml";
constexpr const char* kMetadataFile = "metadata.json";
constexpr const char* kLibsDirectory = "libs";

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

}

TargetKey AndroidPlatformBackend::Key() const {
  return TargetKey{kAndroidPlatformName, kJavaLanguageName};
}

bool AndroidPlatformBackend::Generate(const GenerationInput& input, LanguageBackend& language,
                                      Diagnostics& diagnostics, OutputSink& sink) {
  options_ = input.config;
  diagnostics_ = &diagnostics;
  sink_ = &sink;
  resources_->set_output_prefix(MainPrefix());
  resources_->set_source_prefix(SourceDir());

  GeneratedProgram program = language.Emit(input, diagnostics);
  if (diagnostics.has_errors())
    return false;
  if (input.entry == nullptr) {
    diagnostics.Fatal("当前环境中找不到入口类 \"" + extension_->EntryClassName() + "\"",
                      "output.entry.class-not-found");
    return false;
  }
  return GenerateInternal(input, program);
}

bool AndroidPlatformBackend::WriteFile(const std::string& relative_path,
                                       const std::string& content) {
  if (sink_ == nullptr)
    return false;
  return WriteOutputFile(*sink_, relative_path, content, options_.output_dir, diagnostics_);
}

void AndroidPlatformBackend::PrepareLibraries() {
  library_entries_.clear();
  std::set<std::string> seen;
  const std::vector<const ClassInfo*>& emitted =
      output_classes_ != nullptr ? *output_classes_ : std::vector<const ClassInfo*>();
  const bool hard_mode = options_.Flag(option::kHardMode);
  const std::string libs_dir = LibsDir();
  const auto resolve_library_path = [&](const ClassDecl& declaration, const std::string& path) {
    std::filesystem::path candidate(path);
    if (!candidate.is_absolute()) {
      std::vector<std::string> prefixes;
      for (const std::string& prefix : options_.ListOption(option::kSearchLibPrefix))
        prefixes.push_back(prefix);
      const std::filesystem::path resolved =
          file_search::ResolvePath(path, declaration.file, prefixes);
      if (!resolved.empty())
        return resolved.string();
      std::vector<std::filesystem::path> bases;
      bases.push_back(std::filesystem::path(declaration.file).parent_path());
      bases.push_back(std::filesystem::path("."));
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
  const auto expand_library = [&resolve_library_path, &seen](
                                  const ClassDecl& declaration, const std::string& path,
                                  bool compile_only, std::vector<LibraryEntry>* entries) {
    const std::string file = resolve_library_path(declaration, path);
    std::error_code directory_error;
    if (!std::filesystem::is_directory(file, directory_error)) {
      if (!seen.insert(file).second)
        return;
      LibraryEntry entry;
      entry.file = file;
      entry.compile_only = compile_only;
      entries->push_back(std::move(entry));
      return;
    }
    std::vector<std::filesystem::path> children;
    for (const std::filesystem::directory_entry& item : std::filesystem::directory_iterator(file)) {
      if (!item.is_regular_file())
        continue;
      const std::string extension = item.path().extension().string();
      if (extension != ".jar" && extension != ".aar")
        continue;
      children.push_back(item.path());
    }
    std::sort(children.begin(), children.end());
    for (const std::filesystem::path& child : children) {
      const std::string canonical = std::filesystem::weakly_canonical(child).string();
      if (!seen.insert(canonical).second)
        continue;
      LibraryEntry entry;
      entry.file = canonical;
      entry.compile_only = compile_only;
      entries->push_back(std::move(entry));
    }
  };
  for (const ClassInfo* klass : emitted) {
    if (klass == nullptr || klass->declaration == nullptr)
      continue;
    const ClassDecl& declaration = *klass->declaration;
    const AndroidLibraries* libraries = effects_.Get<AndroidLibraries>(&declaration);
    if (libraries == nullptr)
      continue;
    for (const AndroidLibrarySpec& spec : libraries->entries) {
      if (spec.path.empty())
        continue;
      std::vector<LibraryEntry> expanded;
      expand_library(declaration, spec.path, spec.compile_only, &expanded);
      for (LibraryEntry& entry : expanded) {
        if (hard_mode && !entry.compile_only) {
          const std::string base = std::filesystem::path(entry.file).filename().string();
          const std::string relative = libs_dir + "/" + base;
          if (sink_ != nullptr)
            sink_->CopyFile(entry.file, relative);
          entry.copied = true;
          std::error_code abs_error;
          const std::filesystem::path absolute_root =
              std::filesystem::absolute(options_.output_dir, abs_error);
          entry.file =
              (abs_error ? std::filesystem::path(options_.output_dir) : absolute_root).string() +
              "/" + relative;
        }
        library_entries_.push_back(std::move(entry));
      }
    }
  }
  const bool hard_mode_enabled = options_.Flag(option::kHardMode);
  if (!library_entries_.empty() && sink_ != nullptr && hard_mode_enabled) {
    sink_->CreateDirectory(libs_dir);
  }
}

std::string AndroidPlatformBackend::LibsJson() const {
  if (library_entries_.empty())
    return "[]";
  std::string json = "[\n";
  for (std::size_t i = 0; i < library_entries_.size(); ++i) {
    json += "    {\n";
    json += std::string("      \"compileOnly\": ") +
            (library_entries_[i].compile_only ? "true" : "false") + ",\n";
    json += "      \"file\": \"" + JsonEscape(library_entries_[i].file) + "\"\n";
    json += "    }";
    if (i + 1 != library_entries_.size())
      json += ",";
    json += "\n";
  }
  json += "  ]";
  return json;
}

bool AndroidPlatformBackend::GradleMode() const {
  return options_.Flag(option::kGradle);
}

std::string AndroidPlatformBackend::MainPrefix() const {
  return GradleMode() ? "app/src/main/" : "";
}

std::string AndroidPlatformBackend::SourceDir() const {
  return GradleMode() ? "java/" : "src/";
}

std::string AndroidPlatformBackend::LibsDir() const {
  return GradleMode() ? std::string("app/") + kLibsDirectory : std::string(kLibsDirectory);
}

std::string AndroidPlatformBackend::ReadTemplate(const std::string& relative) const {
  std::ifstream input(std::string(TIECODEC_TEMPLATE_DIR) + "/" + relative, std::ios::binary);
  if (!input.is_open())
    return {};
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

std::string AndroidPlatformBackend::AppBuildGradle() const {
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
  replace_all(&text, "namespace '结绳.中文'",
              "namespace '" + extension_->ResourceNamespace() + "'");
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

bool AndroidPlatformBackend::WriteGradleProject() {
  const auto write_template = [this](const std::string& template_path,
                                     const std::string& output_path,
                                     const std::string& override_content) {
    const std::string content =
        override_content.empty() ? ReadTemplate(template_path) : override_content;
    if (content.empty()) {
      if (diagnostics_ != nullptr) {
        diagnostics_->Fatal("缺少模板文件: " + template_path, "io.template");
      }
      return false;
    }
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

}

bool AndroidPlatformBackend::WriteProjectFiles(const GenerationInput& input,
                                               const GeneratedProgram& program) {
  const ClassInfo& entry = *input.entry;
  const std::string package = options_.package_name;
  const std::string activity = program.entry_qualified_name;
  const auto qualified_name = [&program](const ClassInfo* klass) -> std::string {
    if (klass == nullptr)
      return {};
    const auto it = program.qualified_names.find(klass);
    return it == program.qualified_names.end() ? std::string() : it->second;
  };

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
      const AndroidManifestSpecs* specs = effects_.Get<AndroidManifestSpecs>(target);
      if (specs == nullptr)
        return;
      for (const AndroidManifestEntrySpec& spec : specs->entries) {
        const std::string node =
            spec.node.empty() ? "component.property" : UnquoteAnnotationText(spec.node);
        const std::string content = UnquoteAnnotationText(spec.value);
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
            for (const IfArm& arm : statement->if_arms) {
              walk_expression(arm.condition.get());
              walk_statements(arm.body);
            }
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
    if (klass->HasExternalType("java") || klass->forbid_new)
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
      if (kind.empty() && cursor->HasExternalType("java") &&
          cursor->ExternalType("java").find("Activity") != std::string::npos) {
        kind = "activity";
      }
      if (!kind.empty())
        break;
      if (cursor->declaration->base_name.empty())
        break;
      cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
    }
    const bool is_entry = klass == input.entry;
    if (kind.empty() && !is_entry)
      continue;

    ComponentEntry component;
    component.kind = kind.empty() ? "activity" : kind;
    component.is_entry = is_entry;
    component.qualified = qualified_name(klass);
    if (component.qualified.empty())
      continue;
    component.source_qualified = klass->qualified_name;
    if (const AndroidWindowThemeSpec* theme =
            effects_.Get<AndroidWindowThemeSpec>(klass->declaration)) {
      component.theme = strip_quotes(theme->name);
    }
    if (const AndroidManifestSpecs* specs =
            effects_.Get<AndroidManifestSpecs>(klass->declaration)) {
      for (const AndroidManifestEntrySpec& spec : specs->entries) {
        const std::string node = spec.node.empty() ? "component.property" : strip_quotes(spec.node);
        const std::string value = strip_quotes(spec.value);
        if (value.empty())
          continue;
        if (node == "component.child")
          component.children.push_back(value);
        else if (node == "component.property")
          component.properties.push_back(value);
      }
    }
    collect_call_site_fragments(*klass, &component.call_properties, &component.call_children);
    components.push_back(std::move(component));
  }

  std::sort(components.begin(), components.end(),
            [](const ComponentEntry& a, const ComponentEntry& b) {
              if (a.is_entry != b.is_entry)
                return a.is_entry;
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
      const std::string member_qualified = qualified_name(member_class);
      if (member_qualified == activity) {
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
  const auto add_permissions = [&](const void* node) {
    const AndroidManifestSpecs* specs = effects_.Get<AndroidManifestSpecs>(node);
    if (specs == nullptr)
      return;
    for (const AndroidManifestEntrySpec& spec : specs->entries) {
      if (spec.node != "permission")
        continue;
      std::string permission;
      if (!spec.value.empty() && spec.value.front() == '"') {
        permission = UnquoteAnnotationText(spec.value);
      } else if (!ResolveConstantString(symbols_, entry, spec.value, &permission)) {
        permission = UnquoteAnnotationText(spec.value);
      }
      if (permission.empty())
        continue;
      if (std::find(permissions.begin(), permissions.end(), permission) == permissions.end())
        permissions.push_back(permission);
    }
  };
  for (const ClassInfo* klass : *output_classes_) {
    if (klass == nullptr || klass->declaration == nullptr)
      continue;
    add_permissions(klass->declaration);
    for (const Member& member : klass->declaration->members) {
      if (used_members_ != nullptr && used_members_->count(&member) == 0)
        continue;
      add_permissions(&member);
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
  const bool release_mode = options_.Option(option::kRelease, "0") != "0";
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
  if (!WriteFile(MainPrefix() + kAndroidManifestFile, manifest))
    return false;

  resources_->Write(*sink_);

  PrepareLibraries();
  const std::string& out = options_.output_dir;
  const std::string prefix = MainPrefix();
  std::string metadata;
  metadata += "{\n";
  metadata += "  \"appConfig\": {\n";
  metadata += "    \"appIcon\": \"" + JsonEscape(options_.app_icon) + "\",\n";
  metadata += "    \"appName\": \"" + JsonEscape(options_.app_name) + "\",\n";
  metadata += "    \"minSdk\": " + std::to_string(options_.min_sdk) + ",\n";
  metadata += "    \"targetSdk\": " + std::to_string(options_.target_sdk) + ",\n";
  metadata += "    \"versionCode\": " + std::to_string(options_.version_code) + ",\n";
  metadata += "    \"versionName\": \"" + JsonEscape(options_.version_name) + "\"\n";
  metadata += "  },\n";
  metadata += "  \"libs\": " + LibsJson() + ",\n";
  metadata += "  \"platform\": \"Android\",\n";
  metadata += "  \"productInfo\": {\n";
  metadata += "    \"assetsDir\": \"" + JsonEscape(out + "/" + prefix + "assets") + "\",\n";
  metadata += "    \"extraResDirs\": [],\n";
  metadata += "    \"jniLibsDir\": \"" + JsonEscape(out + "/" + prefix + "jniLibs") + "\",\n";
  metadata += "    \"libsDir\": \"" + JsonEscape(out + "/" + LibsDir()) + "\",\n";
  metadata +=
      "    \"manifest\": \"" + JsonEscape(out + "/" + prefix + "AndroidManifest.xml") + "\",\n";
  metadata += "    \"packageName\": \"" + JsonEscape(package) + "\",\n";
  metadata += "    \"rNamespace\": \"" + JsonEscape(extension_->ResourceNamespace()) + "\",\n";
  metadata += "    \"resDir\": \"" + JsonEscape(out + "/" + prefix + "res") + "\",\n";
  const std::string source_dir = SourceDir();
  metadata += "    \"srcDir\": \"" +
              JsonEscape(out + "/" + prefix + source_dir.substr(0, source_dir.size() - 1)) + "\"\n";
  metadata += "  }\n";
  metadata += "}";
  if (!WriteFile(kMetadataFile, metadata))
    return false;

  const std::string line_map_path = options_.Option(option::kLineMap);
  if (!line_map_path.empty()) {
    std::string mapping_error;
    if (!WriteSourceMapping(line_map_path, source_mapping_, &mapping_error)) {
      if (diagnostics_ != nullptr)
        diagnostics_->Fatal(mapping_error, "io.write");
      return false;
    }
  }
  if (GradleMode() && !WriteGradleProject())
    return false;
  return true;
}

bool AndroidPlatformBackend::GenerateInternal(const GenerationInput& input,
                                              const GeneratedProgram& program) {
  used_members_ = &input.plan.members;
  output_classes_ = &input.plan.classes;
  source_mapping_ = BuildSourceMapping(program);
  for (const GeneratedSource& source : program.sources) {
    if (!WriteFile(MainPrefix() + SourceDir() + source.file_name, source.content))
      return false;
  }
  for (const ClassInfo* klass : input.plan.classes) {
    if (klass == nullptr || klass->declaration == nullptr || klass->forbid_output)
      continue;
    resources_->ProcessClassAnnotations(*klass);
    for (const Member& member : klass->declaration->members) {
      if (input.plan.members.count(&member) == 0)
        continue;
      if (member.kind == MemberKind::kVariable || member.kind == MemberKind::kConstant)
        continue;
      resources_->ProcessMemberManifest(*klass, member);
    }
    for (const Member& member : klass->declaration->members) {
      if (input.plan.members.count(&member) == 0)
        continue;
      if (member.kind != MemberKind::kVariable && member.kind != MemberKind::kConstant)
        continue;
      resources_->ProcessMemberManifest(*klass, member);
    }
  }
  const bool written = WriteProjectFiles(input, program);
  used_members_ = nullptr;
  output_classes_ = nullptr;
  return written;
}

}
