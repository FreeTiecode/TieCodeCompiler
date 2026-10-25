#include "android_java_extension.h"

#include <set>
#include <utility>

#include "android_effect_types.h"
#include "android_resource_binder.h"
#include "constants.h"

namespace tiecodec {
namespace {

std::string SimpleName(const std::string& qualified_name) {
  const std::size_t dot = qualified_name.rfind('.');
  return dot == std::string::npos ? qualified_name : qualified_name.substr(dot + 1);
}

}

AndroidJavaExtension::AndroidJavaExtension(const SymbolTable& symbols, const TargetConfig& config,
                                           const SemanticModel& semantics,
                                           const EffectStore& effects, std::string entry_class_name)
    : symbols_(symbols),
      config_(config),
      semantics_(semantics),
      effects_(effects),
      entry_class_name_(std::move(entry_class_name)),
      resources_(std::make_unique<AndroidResourceBinder>(symbols, config, semantics, effects)) {}

AndroidJavaExtension::~AndroidJavaExtension() = default;

std::string AndroidJavaExtension::ApplicationBase(const ClassInfo& klass) const {
  if (klass.declaration != nullptr &&
      effects_.Get<AndroidApplicationSpec>(klass.declaration) != nullptr) {
    return "android.app.Application";
  }
  return {};
}

std::string AndroidJavaExtension::ComponentContextType() const {
  return "android.content.Context";
}

std::string AndroidJavaExtension::ComponentInitMethod() const {
  return "onInit";
}

std::string AndroidJavaExtension::ComponentCreateMethod() const {
  return "onCreateComponent";
}

std::string AndroidJavaExtension::CreateCompletedMethod() const {
  return kCreateCompletedMember;
}

std::string AndroidJavaExtension::ResourceNamespace() const {
  return config_.Option(option::kRNamespace, config_.namespace_name);
}

bool AndroidJavaExtension::IsComponent(const ClassInfo& klass) const {
  const ClassInfo* cursor = &klass;
  std::set<const ClassInfo*> visited;
  while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
    const std::string simple = SimpleName(cursor->qualified_name);
    if (simple == kWindowComponentClass || simple == kVisualComponentClass ||
        simple == kLayoutComponentClass)
      return true;
    if (cursor->declaration->base_name.empty())
      break;
    cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
  }
  return false;
}

bool AndroidJavaExtension::IsComponentContainer(const ClassInfo& klass) const {
  const ClassInfo* cursor = &klass;
  std::set<const ClassInfo*> visited;
  while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
    const std::string simple = SimpleName(cursor->qualified_name);
    if (simple == kComponentContainerClass)
      return true;
    if (cursor->declaration->base_name.empty())
      break;
    cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
  }
  return false;
}

bool AndroidJavaExtension::IsEntryClass(const ClassInfo& klass) const {
  return klass.declaration != nullptr && klass.declaration->name == entry_class_name_;
}

bool AndroidJavaExtension::IsComponentField(const Member& member) const {
  return effects_.Get<AndroidComponentSpec>(&member) != nullptr;
}

std::vector<ComponentField> AndroidJavaExtension::CollectComponents(const ClassInfo& klass) const {
  std::vector<ComponentField> fields;
  if (klass.declaration == nullptr)
    return fields;
  for (const Member& member : klass.declaration->members) {
    const AndroidComponentSpec* spec = effects_.Get<AndroidComponentSpec>(&member);
    if (spec == nullptr)
      continue;
    ComponentField field;
    field.member = &member;
    field.parent = spec->parent;
    field.is_root = spec->root;

    const std::string& table = spec->table;
    std::size_t i = 0;
    if (i < table.size() && table[i] == '{')
      ++i;
    std::string key;
    std::string value;
    bool in_key = true;
    int depth = 0;
    const auto flush = [&]() {
      if (!key.empty()) {
        std::string trimmed_key = key;
        while (!trimmed_key.empty() && (trimmed_key.back() == ' ' || trimmed_key.back() == '\t')) {
          trimmed_key.pop_back();
        }
        std::size_t begin = 0;
        while (begin < trimmed_key.size() &&
               (trimmed_key[begin] == ' ' || trimmed_key[begin] == '\t')) {
          ++begin;
        }
        std::string property_name = trimmed_key.substr(begin);
        bool parent_property = false;
        if (!property_name.empty() && property_name[0] == '$') {
          parent_property = true;
          property_name = property_name.substr(1);
        }
        field.properties.push_back(ComponentProperty{property_name, value, parent_property});
      }
      key.clear();
      value.clear();
      in_key = true;
    };
    for (; i < table.size(); ++i) {
      const char c = table[i];
      if (c == '(' || c == '[' || c == '{')
        ++depth;
      if (c == ')' || c == ']' || c == '}') {
        if (depth == 0)
          break;
        --depth;
      }
      if (in_key && c == '=') {
        in_key = false;
        continue;
      }
      if (!in_key && c == ',' && depth == 0) {
        flush();
        continue;
      }
      if (in_key) {
        key.push_back(c);
      } else {
        value.push_back(c);
      }
    }
    flush();

    if (spec->layout) {
      std::vector<ComponentProperty> rest;
      for (auto& property : field.properties) {
        if (property.name == "根布局") {
          field.is_root = property.value == "真";
          continue;
        }
        if (property.name == "父布局") {
          field.parent = property.value;
          continue;
        }
        if (!property.name.empty() && property.name[0] == '@') {
          property.name = property.name.substr(1);
          property.parent = true;
        }
        rest.push_back(property);
      }
      field.properties = std::move(rest);
    }
    fields.push_back(std::move(field));
  }
  return fields;
}

std::string AndroidJavaExtension::ComponentReturnType(const ClassInfo& klass) const {
  const ClassInfo* cursor = &klass;
  std::set<const ClassInfo*> visited;
  const auto inspect = [&](const std::string& code) -> std::string {
    std::size_t position = 0;
    while ((position = code.find("onCreateComponent", position)) != std::string::npos) {
      const std::size_t line_begin = code.rfind('\n', position);
      const std::string line =
          code.substr(line_begin == std::string::npos ? 0 : line_begin + 1,
                      position - (line_begin == std::string::npos ? 0 : line_begin + 1));

      const std::size_t cls = line.find("#cls<");
      if (cls != std::string::npos) {
        const std::size_t close = line.find('>', cls);
        if (close != std::string::npos)
          return line.substr(cls + 5, close - cls - 5);
      }
      const std::size_t ncls = line.find("#ncls<");
      if (ncls != std::string::npos) {
        const std::size_t close = line.find('>', ncls);
        if (close != std::string::npos)
          return line.substr(ncls + 6, close - ncls - 6);
      }

      for (const char* candidate :
           {kLayoutComponentClass, kVisualComponentClass, kWindowComponentClass}) {
        if (line.find(candidate) != std::string::npos)
          return candidate;
      }
      position += 1;
    }
    return {};
  };
  while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
    for (const Member& member : cursor->declaration->members) {
      if (!member.code_text.empty()) {
        const std::string type = inspect(member.code_text);
        if (!type.empty())
          return type;
      }
      for (const StmtPtr& statement : member.body) {
        if (statement == nullptr || statement->kind != StmtKind::kCode)
          continue;
        const std::string type = inspect(statement->code_text);
        if (!type.empty())
          return type;
      }
    }
    if (cursor->declaration->base_name.empty())
      break;
    cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
  }
  return kLayoutComponentClass;
}

std::string AndroidJavaExtension::ComponentAddMethod() const {
  return "添加组件";
}

std::string AndroidJavaExtension::ComponentSetLayoutMethod() const {
  return "setLayout";
}

bool AndroidJavaExtension::EmitsAutoSubscriptions(const ClassInfo& klass) const {
  return IsComponent(klass) || IsEntryClass(klass);
}

ResourceBinder* AndroidJavaExtension::Resources() const {
  return resources_.get();
}

bool AndroidJavaExtension::IsResourceValueRule(const std::string& rule) const {
  return rule == annotation::kRuleDrawable || rule == annotation::kRuleDrawableXxh ||
         rule == annotation::kRuleAnim || rule == annotation::kRuleXml;
}

}
