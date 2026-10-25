#include "java_emitter.h"

#include "constants.h"
#include "java_const.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>

#include "internal/helpers.h"
#include "local_use.h"
#include "symbol_resolution.h"

namespace tiecodec {

using namespace detail;

JavaEmitter::JavaEmitter(const SymbolTable& symbols, const BackendConfig& config,
                         const NamingScheme& names, TypeMapper& types, Diagnostics& diagnostics,
                         ResourceBinder* resources)
    : symbols_(symbols),
      config_(config),
      names_(names),
      types_(types),
      diagnostics_(diagnostics),
      resources_(resources) {
  if (IsPinyinNaming(config.friendly_name))
    generated_name_counter_ = kPinyinGeneratedNameStart;
}

void JavaEmitter::SetPlan(
    const std::set<const Member*>& used_members,
    const std::vector<std::pair<const ClassInfo*, const Member*>>& virtual_overrides,
    const std::vector<const ClassInfo*>& output_classes) {
  used_members_ = &used_members;
  virtual_overrides_ = &virtual_overrides;
  output_classes_ = &output_classes;
}
ClassOutput JavaEmitter::EmitClass(const ClassInfo& klass) {
  call_site_imports_.clear();
  current_file_ = klass.declaration != nullptr ? klass.declaration->file : std::string();
  const int class_line =
      klass.declaration != nullptr && klass.declaration->declaration_line > 0
          ? klass.declaration->declaration_line
          : (klass.declaration != nullptr ? static_cast<int>(klass.declaration->position.line) + 1
                                          : 0);
  if (IsGenericClass(klass)) {
    std::set<std::string> shell_imports;
    std::string shell_base;
    if (klass.declaration != nullptr && !klass.declaration->base_name.empty()) {
      std::string base_name;
      std::vector<std::string> base_arguments;
      const ClassInfo* base_class = nullptr;
      if (SplitGenericType(klass.declaration->base_name, &base_name, &base_arguments)) {
        base_class = symbols_.Resolve(base_name, &klass);
      } else {
        base_class = symbols_.Resolve(klass.declaration->base_name, &klass);
      }
      shell_base = names_.BaseName(klass);
      if (base_class != nullptr && base_class != &klass && !klass.has_java_alias &&
          !base_class->has_java_alias) {
        shell_imports.insert(names_.ImportPath(*base_class));
      }
      if (klass.has_java_alias)
        shell_base.clear();
    }
    shell_imports.insert(config_.Option(option::kRNamespace, config_.namespace_name) + ".R");
    EmittedText shell;
    shell.Mark(current_file_, class_line);
    shell += "package " + names_.Package(klass) + ";\n\n";
    for (const std::string& import : shell_imports)
      shell += "import " + import + ";\n";
    if (shell_imports.size() > 1)
      shell += "\n";
    shell += "public class " + names_.ClassName(klass);
    if (!shell_base.empty())
      shell += " extends " + shell_base;
    shell += " {\n}";
    const int shell_end =
        klass.declaration != nullptr && klass.end_ref_line > 0 ? klass.end_ref_line : class_line;
    shell.Mark(current_file_, shell_end);
    shell.Finish();
    ClassOutput shell_output;
    shell_output.text = shell;
    shell_output.line_refs = shell.line_refs();
    return shell_output;
  }
  std::set<std::string> imports;
  CollectImports(klass, &imports);
  EmittedText body;
  pending_async_machines_.clear();
  body.Mark(current_file_, class_line);
  if (klass.declaration != nullptr) {
    bool previous_was_code_block = false;
    const auto emit_member = [&](const Member& member) {
      const std::size_t before = body.size();
      suppress_code_block_blank_ = previous_was_code_block;
      EmitMember(klass, member, &body);
      if (body.size() != before)
        previous_was_code_block = member.kind == MemberKind::kCodeBlock;
    };
    for (const Member& member : klass.declaration->members) {
      if (member.kind == MemberKind::kCodeBlock)
        continue;
      emit_member(member);
    }
    if (virtual_overrides_ != nullptr) {
      for (const auto& entry : *virtual_overrides_) {
        if (entry.first != &klass || entry.second == nullptr)
          continue;
        const Member& member = *entry.second;
        const std::string return_type =
            member.type_name.empty() ? "void" : JavaTypeOf(klass, member.type_name);
        std::string parameters;
        for (const Parameter& parameter : member.parameters) {
          if (!parameters.empty())
            parameters += ", ";
          parameters +=
              (parameter.type_name.empty() ? "Object" : JavaTypeOf(klass, parameter.type_name)) +
              " " + names_.LocalName(parameter.name);
        }
        body.Mark(current_file_, class_line);
        body += "\n  public " + return_type + " " + names_.MemberName(member) + "(" + parameters +
                ") {\n  }\n";
        previous_was_code_block = false;
      }
    }
  }
  for (const std::string& machine : pending_async_machines_)
    body += machine;
  pending_async_machines_.clear();
  std::vector<const Member*> trailing_code_blocks;
  if (klass.declaration != nullptr) {
    for (const Member& member : klass.declaration->members) {
      if (member.kind == MemberKind::kCodeBlock)
        trailing_code_blocks.push_back(&member);
    }
  }

  const std::vector<ComponentField> component_fields = CollectComponentFields(klass);
  const bool is_entry =
      (klass.declaration != nullptr && klass.declaration->name == kEntryClassName) ||
      klass.qualified_name == std::string(config_.namespace_name) + "." + kEntryClassName;
  const bool is_container = IsComponentContainerClass(klass);
  if (is_container && !component_fields.empty()) {
    EmitOnCreateComponent(klass, component_fields, &body);
  } else if (is_entry || !component_fields.empty()) {
    body.Mark(current_file_, class_line);
    body += "\n  public void onInit() {\n";
    EmitComponentInit(klass, component_fields, &body, names_.ClassName(klass) + ".this", "");
    body += "  }\n";
  }

  if (!klass.has_java_alias && IsComponentClass(klass) && !HasDeclaredConstructor(klass)) {
    bool dispatch_create_done = false;
    std::string dispatch_create_name;
    if (klass.declaration != nullptr) {
      for (const Member& member : klass.declaration->members) {
        if (member.kind == MemberKind::kEventBlock && member.name == kCreateCompletedMember) {
          dispatch_create_done = true;
          dispatch_create_name = names_.MemberName(member);
        }
      }
    }
    if (!dispatch_create_done && virtual_overrides_ != nullptr) {
      for (const auto& entry : *virtual_overrides_) {
        if (entry.first != &klass || entry.second == nullptr)
          continue;
        if (entry.second->name == kCreateCompletedMember) {
          dispatch_create_done = true;
          dispatch_create_name = names_.MemberName(*entry.second);
        }
      }
    }
    if (dispatch_create_done) {
      body.Mark(current_file_, class_line);
      body += "\n  public " + names_.ClassName(klass) + "(android.content.Context context) {\n";
      body += "    super(context);\n";
      body += "    " + dispatch_create_name + "();\n";
      body += "  }\n";
    }
  }

  {
    bool previous_was_code_block = false;
    for (const Member* member : trailing_code_blocks) {
      suppress_code_block_blank_ = previous_was_code_block;
      const std::size_t before = body.size();
      EmitMember(klass, *member, &body);
      if (body.size() != before)
        previous_was_code_block = true;
    }
    suppress_code_block_blank_ = false;
  }

  imports.insert(call_site_imports_.begin(), call_site_imports_.end());
  call_site_imports_.clear();
  EmittedText out;
  out.Mark(current_file_, class_line);
  out += "package " + names_.Package(klass) + ";\n\n";
  for (const std::string& import : imports) {
    out += "import " + import + ";\n";
  }
  std::string base;
  if (klass.declaration != nullptr && !klass.declaration->base_name.empty())
    base = MapGenericAwareType(klass, klass.declaration->base_name, false, false);
  bool global_application = false;
  if (klass.declaration != nullptr) {
    for (const Annotation& annotation : klass.declaration->annotations) {
      if (annotation.name == "全局应用")
        global_application = true;
    }
  }
  if (global_application) {
    base = "android.app.Application";
  } else if (klass.has_java_alias) {
    base.clear();
  }
  if (imports.size() > 1)
    out += "\n";
  std::string class_prefix;
  std::string class_suffix;
  if (klass.declaration != nullptr) {
    for (const Annotation& annotation : klass.declaration->annotations) {
      const std::string value = annotation.TextArgument(kArgumentParameter);
      if (annotation.name == annotation::kPrefixCode)
        class_prefix = value;
      if (annotation.name == "后缀代码")
        class_suffix = value;
    }
  }
  out += "public ";
  if (!class_prefix.empty())
    out += class_prefix + " ";
  out += "class " + names_.ClassName(klass);
  if (!base.empty())
    out += " extends " + base;
  if (!class_suffix.empty())
    out += " " + class_suffix;
  out += " {\n";
  out.AppendText(body);
  const int end_ref_line = klass.declaration != nullptr ? klass.end_ref_line : 0;
  out.Mark(current_file_, end_ref_line > 0 ? end_ref_line : class_line);
  out += "}";
  out.Finish();

  emitted_.insert(&klass);
  ClassOutput result;
  result.text = out;
  result.line_refs = out.line_refs();
  return result;
}
std::vector<JavaEmitter::ComponentField> JavaEmitter::CollectComponentFields(
    const ClassInfo& klass) const {
  std::vector<ComponentField> fields;
  if (klass.declaration == nullptr)
    return fields;
  for (const Member& member : klass.declaration->members) {
    const Annotation* config = nullptr;
    bool tly_config = false;
    for (const Annotation& annotation : member.annotations) {
      if (annotation.name == annotation::kComponentConfig)
        config = &annotation;
      if (annotation.name == annotation::kLayoutConfig) {
        config = &annotation;
        tly_config = true;
      }
    }
    if (config == nullptr)
      continue;
    ComponentField field;
    field.member = &member;
    for (const AnnotationArgument& argument : config->arguments) {
      if (argument.name == annotation::kRootLayout) {
        field.is_root = argument.value == "真";
        continue;
      }
      if (argument.name == annotation::kParentLayout) {
        field.parent = argument.value;
        continue;
      }
      if (!argument.name.empty())
        continue;

      const std::string& table = argument.value;
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
          while (!trimmed_key.empty() &&
                 (trimmed_key.back() == ' ' || trimmed_key.back() == '\t')) {
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
    }
    if (tly_config) {
      std::vector<ComponentProperty> rest;
      for (auto& property : field.properties) {
        if (property.name == annotation::kRootLayout) {
          field.is_root = property.value == "真";
          continue;
        }
        if (property.name == annotation::kParentLayout) {
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

void JavaEmitter::EmitComponentInit(const ClassInfo& klass,
                                    const std::vector<ComponentField>& fields, EmittedText* body,
                                    const std::string& window_argument,
                                    const std::string& return_statement) {
  if (fields.empty())
    return;
  const std::string window = window_argument;
  const auto member_name = [this](const Member& member) { return names_.MemberName(member); };
  const auto translate_member = [this, &klass](const ClassInfo* owner,
                                               const std::string& name) -> std::string {
    const ClassInfo* member_owner = nullptr;
    const ClassInfo* cursor = &klass;
    std::set<const ClassInfo*> visited;
    while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
      for (const Member& member : cursor->declaration->members) {
        if (member.name == name)
          return names_.MemberName(member);
      }
      if (cursor->declaration->base_name.empty())
        break;
      cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
    }
    if (owner != nullptr) {
      if (const Member* member = FindMemberInChain(symbols_, *owner, name, &member_owner)) {
        return names_.MemberName(*member);
      }
    }
    return name;
  };
  for (const ComponentField& field : fields) {
    if (field.member == nullptr)
      continue;
    const std::string type = JavaTypeOf(klass, field.member->type_name);
    body->append("    ");
    body->append(member_name(*field.member) + " = new " + type + "(" + window + ");\n");
  }
  for (const ComponentField& field : fields) {
    if (field.member == nullptr)
      continue;
    const ClassInfo* field_type = symbols_.Resolve(field.member->type_name, &klass);
    if (!field.parent.empty()) {
      const std::string parent_name = translate_member(nullptr, field.parent);
      const ClassInfo* parent_type = nullptr;
      if (const Member* parent = FindMemberInChain(symbols_, klass, field.parent, nullptr)) {
        parent_type = symbols_.Resolve(parent->type_name, &klass);
      }
      const std::string add_method = translate_member(parent_type, "添加组件");
      body->append("    " + parent_name + "." + add_method + "(" + member_name(*field.member) +
                   ");\n");
    }
    for (const auto& property : field.properties) {
      std::string value = property.value;
      if (value == "真")
        value = "true";
      if (value == "假")
        value = "false";
      if (property.parent) {
        const std::string parent_name = translate_member(nullptr, field.parent);
        const ClassInfo* parent_type = nullptr;
        if (const Member* parent_member =
                FindMemberInChain(symbols_, klass, field.parent, nullptr)) {
          parent_type = symbols_.Resolve(parent_member->type_name, &klass);
        }
        const std::string method_name = translate_member(parent_type, property.name);
        body->append("    " + parent_name + "." + method_name + "(" + member_name(*field.member) +
                     ", " + value + ");\n");
      } else {
        const std::string property_name = translate_member(field_type, property.name);
        body->append("    " + member_name(*field.member) + "." + property_name + "(" + value +
                     ");\n");
      }
    }
  }
  if (!return_statement.empty()) {
    for (const ComponentField& field : fields) {
      if (field.member == nullptr || !field.is_root)
        continue;
      body->append("    " + return_statement + " " + member_name(*field.member) + ";\n");
    }
    return;
  }
  for (const ComponentField& field : fields) {
    if (field.member == nullptr || !field.is_root)
      continue;
    body->append("    setLayout(" + member_name(*field.member) + ");\n");
  }
}

std::string JavaEmitter::OnCreateComponentReturnTypeName(const ClassInfo& klass) const {
  const ClassInfo* cursor = &klass;
  std::set<const ClassInfo*> visited;
  const auto inspect = [&](const ClassInfo& owner, const std::string& code) -> std::string {
    std::size_t position = 0;
    while ((position = code.find("onCreateComponent", position)) != std::string::npos) {
      const std::size_t line_begin = code.rfind('\n', position);
      const std::string line =
          code.substr(line_begin == std::string::npos ? 0 : line_begin + 1,
                      position - (line_begin == std::string::npos ? 0 : line_begin + 1));

      const std::size_t cls = line.find("#cls<");
      if (cls != std::string::npos) {
        const std::size_t close = line.find('>', cls);
        if (close != std::string::npos) {
          return line.substr(cls + 5, close - cls - 5);
        }
      }
      const std::size_t ncls = line.find("#ncls<");
      if (ncls != std::string::npos) {
        const std::size_t close = line.find('>', ncls);
        if (close != std::string::npos) {
          return line.substr(ncls + 6, close - ncls - 6);
        }
      }

      for (const char* candidate :
           {kLayoutComponentClass, kVisualComponentClass, kWindowComponentClass}) {
        if (line.find(candidate) != std::string::npos)
          return candidate;
      }
      position += 1;
    }
    (void)owner;
    return {};
  };
  while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
    for (const Member& member : cursor->declaration->members) {
      if (!member.code_text.empty()) {
        const std::string type = inspect(*cursor, member.code_text);
        if (!type.empty())
          return type;
      }
      for (const StmtPtr& statement : member.body) {
        if (statement == nullptr || statement->kind != StmtKind::kCode)
          continue;
        const std::string type = inspect(*cursor, statement->code_text);
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

bool JavaEmitter::EmitOnCreateComponent(const ClassInfo& klass,
                                        const std::vector<ComponentField>& fields,
                                        EmittedText* body) {
  if (fields.empty())
    return false;
  const std::string return_type = types_.MapType(OnCreateComponentReturnTypeName(klass), &klass);
  const std::string context_name = names_.LocalName("context");
  *body += "\n  public " + return_type + " onCreateComponent(android.content.Context " +
           context_name + ") {\n";
  EmitComponentInit(klass, fields, body, context_name, "return");
  *body += "  }\n";
  return true;
}

void JavaEmitter::EmitMember(const ClassInfo& klass, const Member& member, EmittedText* body) {
  if (member.kind != MemberKind::kCodeBlock && member.position.line > 0) {
    body->Mark(current_file_, static_cast<int>(member.position.line) + 1);
  }
  {
    const ClassInfo* chain = &klass;
    std::set<const ClassInfo*> visited;
    while (chain != nullptr && chain->declaration != nullptr && visited.insert(chain).second) {
      for (const Member& chain_member : chain->declaration->members) {
        renames_[chain_member.name] = names_.MemberName(chain_member);
      }
      if (chain->declaration->base_name.empty())
        break;
      chain = symbols_.Resolve(chain->declaration->base_name, chain);
    }
  }
  if (klass.declaration == nullptr)
    return;

  const bool basic_alias_class = klass.has_java_alias;
  const bool emittable_alias_member = member.kind == MemberKind::kMethod ||
                                      member.kind == MemberKind::kPropertyRead ||
                                      member.kind == MemberKind::kPropertyWrite;
  if (klass.has_java_alias && member.kind != MemberKind::kCodeBlock &&
      !(basic_alias_class && emittable_alias_member)) {
    return;
  }

  if (used_members_ != nullptr && used_members_->count(&member) == 0)
    return;

  current_return_type_ = member.type_name;

  const bool inline_code =
      std::any_of(member.annotations.begin(), member.annotations.end(),
                  [](const Annotation& annotation) { return annotation.name == "嵌入式代码"; });
  if (inline_code)
    return;

  switch (member.kind) {
    case MemberKind::kCodeBlock: {
      if (member.code_text.empty())
        return;
      if (!suppress_code_block_blank_)
        *body += "\n";
      const std::string resolved = SubstituteCode(klass, member.code_text, {});
      int code_offset = 0;
      for (const std::string& line :
           NormalizeCodeBlock(resolved, kClassMemberIndentLevel, member.code_indent)) {
        body->Mark(
            current_file_,
            (member.code_first_line > 0 ? member.code_first_line : member.code_line) + code_offset);
        ++code_offset;
        body->append(line);
        body->append("\n");
      }
      return;
    }
    case MemberKind::kMethod:
    case MemberKind::kPropertyRead:
    case MemberKind::kPropertyWrite:
    case MemberKind::kEventBlock: {
      if (IsAsyncMember(member)) {
        EmitAsyncMember(klass, member, body);
        return;
      }

      const bool basic_alias = klass.has_java_alias;
      if (basic_alias) {
        const std::string return_type =
            member.type_name.empty() ? "void" : JavaTypeOf(klass, member.type_name);
        std::string receiver_type = klass.java_alias;
        if (spec_base_ == &klass && !spec_arguments_.empty()) {
          std::string type_name = klass.qualified_name + "<";
          for (std::size_t i = 0; i < spec_arguments_.size(); ++i) {
            if (i > 0)
              type_name += ",";
            type_name += spec_arguments_[i];
          }
          type_name += ">";
          receiver_type = MapGenericAwareType(klass, type_name, true, false);
        }
        std::string parameters = receiver_type + " __this_obj";
        std::string argument_names;
        for (const Parameter& parameter : member.parameters) {
          const std::string parameter_name = names_.LocalName(parameter.name);
          if (!argument_names.empty())
            argument_names += ", ";
          argument_names += parameter_name;
          parameters += ", ";
          parameters +=
              (parameter.type_name.empty() ? "Object" : JavaTypeOf(klass, parameter.type_name)) +
              " " + parameter_name;
        }
        *body += "\n  ";
        if (!HasAnnotationMember(member, annotation::kHidden))
          *body += "public ";
        *body +=
            "static " + return_type + " " + names_.MemberName(member) + "(" + parameters + ") {\n";
        alias_static_context_ = true;

        EmitMemberBody(klass, member, body);
        alias_static_context_ = false;
        body->Mark(current_file_, member.end_line > 0 ? member.end_line : member.declaration_line);
        *body += "  }\n";
        return;
      }
      const std::string return_type =
          member.type_name.empty() ? "void" : JavaTypeOf(klass, member.type_name);
      std::string parameters;
      for (const Parameter& parameter : member.parameters) {
        if (!parameters.empty())
          parameters += ", ";
        const std::string type =
            parameter.type_name.empty() ? "Object" : JavaTypeOf(klass, parameter.type_name);
        parameters += type + " " + names_.LocalName(parameter.name);
      }
      const bool is_static = std::any_of(
          member.annotations.begin(), member.annotations.end(),
          [](const Annotation& annotation) { return annotation.name == annotation::kStatic; });
      *body += "\n  ";
      if (!HasAnnotationMember(member, annotation::kHidden))
        *body += "public ";
      const std::string member_prefix = MemberCodeAnnotation(member, annotation::kPrefixCode);
      const std::string member_suffix = MemberCodeAnnotation(member, "后缀代码");
      if (!member_prefix.empty()) {
        *body += member_prefix;
        *body += " ";
      }
      if (is_static)
        *body += "static ";
      std::string method_name = names_.MemberName(member);
      if (member.kind == MemberKind::kEventBlock && !member.event_object.empty() &&
          symbols_.Resolve(member.event_object, &klass) == nullptr) {
        const bool pinyin = IsPinyinNaming(config_.friendly_name);
        const std::string object_name = renames_.count(member.event_object) != 0
                                            ? renames_[member.event_object]
                                            : member.event_object;
        method_name = object_name + "$" + (pinyin ? PinyinCamel(member.name) : member.name);
      }
      *body += return_type + " " + method_name + "(" + parameters + ")";
      if (!member_suffix.empty())
        *body += " " + member_suffix;
      *body += " {\n";
      EmitMemberBody(klass, member, body);
      body->Mark(current_file_, member.end_line > 0 ? member.end_line : member.declaration_line);
      *body += "  }\n";
      return;
    }
    case MemberKind::kVariable:
    case MemberKind::kConstant: {
      if (resources_ != nullptr && resources_->IsResourceField(klass, member)) {
        const bool resource_static = std::any_of(
            member.annotations.begin(), member.annotations.end(),
            [](const Annotation& annotation) { return annotation.name == annotation::kStatic; });
        *body += "  ";
        if (!HasAnnotationMember(member, annotation::kHidden))
          *body += "public ";
        if (resource_static)
          *body += "static ";
        *body += resources_->FieldType(klass, member) + " " + names_.MemberName(member) + " = " +
                 resources_->BindField(klass, member) + ";\n";
        return;
      }

      const std::string type =
          member.type_name.empty() ? "Object" : JavaTypeOf(klass, member.type_name);
      const bool static_field = std::any_of(
          member.annotations.begin(), member.annotations.end(),
          [](const Annotation& annotation) { return annotation.name == annotation::kStatic; });
      *body += "  ";
      if (!HasAnnotationMember(member, annotation::kHidden))
        *body += "public ";
      const std::string field_prefix = MemberCodeAnnotation(member, annotation::kPrefixCode);
      if (!field_prefix.empty()) {
        *body += field_prefix;
        *body += " ";
      }
      if (static_field)
        *body += "static ";
      if (member.kind == MemberKind::kConstant)
        *body += "final ";
      *body += type + " " + names_.MemberName(member);
      if (member.expression_body != nullptr) {
        *body += " = " + InitializerExpression(klass, member.type_name, *member.expression_body);
      } else if (!member.code_text.empty()) {
        *body += " = " + Trim(member.code_text);
      } else {
        const TypeShape shape = ParseTypeShape(member.type_name);
        if (shape.dimensions > 0) {
          if (!shape.capacities.empty()) {
            *body += " = new " + MapGenericAwareType(klass, shape.base, true, false);
            for (int i = 0; i < shape.dimensions; ++i) {
              if (i < static_cast<int>(shape.capacities.size()) && !shape.capacities[i].empty()) {
                *body += "[" + shape.capacities[i] + "]";
              } else {
                *body += "[]";
              }
            }
          }
        } else {
          const std::string base_type = member.type_name.substr(0, member.type_name.find('?'));
          const ClassInfo* basic = symbols_.Resolve(base_type, &klass);
          bool primitive_alias = false;
          if (basic != nullptr && basic->has_java_alias) {
            const std::string& alias = basic->java_alias;
            primitive_alias = alias == "boolean" || alias == "int" || alias == "long" ||
                              alias == "float" || alias == "double" || alias == "byte" ||
                              alias == "short" || alias == "char";
            if (member.kind != MemberKind::kConstant) {
              if (alias == "boolean")
                *body += " = false";
              else if (alias == "int")
                *body += " = 0";
              else if (alias == "long")
                *body += " = 0L";
              else if (alias == "float")
                *body += " = 0.0F";
              else if (alias == "double")
                *body += " = 0.0";
              else if (alias == "byte")
                *body += " = 0";
            }
            if ((alias == "java.lang.String" || alias == "String") &&
                member.type_name.find('?') == std::string::npos) {
              *body += " = \"\"";
            }
          }

          const bool has_component_config =
              std::any_of(member.annotations.begin(), member.annotations.end(),
                          [](const Annotation& annotation) {
                            return annotation.name == annotation::kComponentConfig ||
                                   annotation.name == annotation::kLayoutConfig;
                          });
          const bool nullable = member.type_name.find('?') != std::string::npos;
          const ClassInfo* field_class = symbols_.Resolve(member.type_name, &klass);
          if (!has_component_config && !nullable && field_class != nullptr &&
              !field_class->forbid_new &&
              (!field_class->has_java_alias ||
               (!primitive_alias && field_class->java_alias != "java.lang.String" &&
                field_class->java_alias != "String"))) {
            *body += " = new " + type + "()";
          }
        }
      }
      *body += ";\n";
      return;
    }
    case MemberKind::kEventDefinition: {
      if (member.name.empty())
        return;
      const bool pinyin_event = IsPinyinNaming(config_.friendly_name);
      const std::string event_part = pinyin_event ? PinyinCamel(member.name) : member.name;
      const std::string listener = event_part + "Listener";
      const std::string store = names_.MemberName(member) + "_v";
      std::string parameters;
      std::string argument_names;
      for (const Parameter& parameter : member.parameters) {
        const std::string parameter_name = names_.LocalName(parameter.name);
        if (!parameters.empty())
          parameters += ", ";
        parameters +=
            (parameter.type_name.empty() ? "Object" : JavaTypeOf(klass, parameter.type_name)) +
            " " + parameter_name;
        if (!argument_names.empty())
          argument_names += ", ";
        argument_names += parameter_name;
      }
      const std::string return_type =
          member.type_name.empty() ? "void" : JavaTypeOf(klass, member.type_name);
      const bool has_return = !member.type_name.empty();

      *body += "\n  ";
      if (!HasAnnotationMember(member, annotation::kHidden))
        *body += "public ";
      *body += return_type + " " + names_.MemberName(member) + "(" + parameters + ") {\n";
      *body += "    if (" + store + " != null) {\n";
      *body += std::string("      ") + (has_return ? "return " : "") + store + ".dispatch(" +
               argument_names + ");\n";
      *body += "    }\n";
      if (has_return) {
        std::string default_value = "null";
        const ClassInfo* return_class = symbols_.Resolve(member.type_name, &klass);
        if (return_class != nullptr && return_class->has_java_alias) {
          if (return_class->java_alias == "boolean")
            default_value = "false";
          else if (return_class->java_alias == "int")
            default_value = "0";
          else if (return_class->java_alias == "long")
            default_value = "0L";
          else if (return_class->java_alias == "float")
            default_value = "0.0F";
          else if (return_class->java_alias == "double")
            default_value = "0.0";
        }
        *body += "    return " + default_value + ";\n";
      }
      *body += "  }\n";

      *body += "\n  public interface " + listener + " {\n";
      *body += "    " + return_type + " dispatch(" + parameters + ");\n";
      *body += "  }\n";
      *body += "  private " + listener + " " + store + ";\n";

      *body += "\n  public void set" + listener + "(" + listener + " receiver) {\n";
      *body += "    this." + store + " = receiver;\n";
      *body += "  }\n";
      return;
    }
  }
}

void JavaEmitter::EmitMemberBody(const ClassInfo& klass, const Member& member, EmittedText* body) {
  local_types_.clear();
  current_parameters_.clear();
  current_parameter_types_ = member.parameters;
  for (const Parameter& parameter : member.parameters) {
    if (!parameter.type_name.empty())
      local_types_[parameter.name] = parameter.type_name;
    current_parameters_[parameter.name] = names_.LocalName(parameter.name);
  }
  if (member.expression_body != nullptr) {
    body->append("    ");
    if (!member.type_name.empty())
      body->append("return ");
    body->append(EmitExpression(klass, *member.expression_body));
    body->append(";\n");
    return;
  }
  if (!member.code_text.empty()) {
    current_code_line_ = member.code_line;
    const std::string resolved = SubstituteCode(klass, member.code_text, current_parameters_);
    current_code_line_ = 0;
    int code_offset = 0;
    for (const std::string& line :
         NormalizeCodeBlock(resolved, kMethodBodyIndentLevel, member.code_indent)) {
      body->Mark(
          current_file_,
          (member.code_first_line > 0 ? member.code_first_line : member.code_line) + code_offset);
      ++code_offset;
      body->append(line);
      body->append("\n");
    }
    return;
  }
  {
    const ClassInfo* chain = &klass;
    std::set<const ClassInfo*> visited;
    while (chain != nullptr && chain->declaration != nullptr && visited.insert(chain).second) {
      for (const Member& chain_member : chain->declaration->members) {
        renames_[chain_member.name] = names_.MemberName(chain_member);
      }
      if (chain->declaration->base_name.empty())
        break;
      chain = symbols_.Resolve(chain->declaration->base_name, chain);
    }
  }
  for (const Parameter& parameter : member.parameters) {
    if (parameter.name.empty())
      continue;
    const std::string translated = names_.LocalName(parameter.name);
    renames_[parameter.name] = translated;
    current_parameters_[parameter.name] = translated;
  }
  referenced_locals_ = UsedLocalNames(member);
  subscriptions_emitted_ = false;
  for (const StmtPtr& statement : member.body) {
    EmitStatement(klass, *statement, kMethodBodyIndentLevel, body);
  }

  if (!subscriptions_emitted_ && member.name == kCreateCompletedMember) {
    EmitSubscriptions(klass, kMethodBodyIndentLevel, body);
  }
  subscriptions_emitted_ = false;
}

bool JavaEmitter::EmitSubscriptions(const ClassInfo& klass, int indent, EmittedText* body) {
  if (klass.declaration == nullptr)
    return false;
  bool emitted = false;
  for (const Member& block : klass.declaration->members) {
    if (block.kind != MemberKind::kEventBlock || block.name.empty() || block.event_object.empty()) {
      continue;
    }

    std::string type_name = MemberTypeOf(klass, block.event_object);
    if (type_name.empty()) {
      const auto local = local_types_.find(block.event_object);
      if (local != local_types_.end())
        type_name = local->second;
    }
    const ClassInfo* receiver_class = type_name.empty()
                                          ? symbols_.Resolve(block.event_object, &klass)
                                          : symbols_.Resolve(type_name, &klass);
    if (receiver_class == nullptr)
      continue;

    const ClassInfo* owner = nullptr;
    const Member* event_definition = nullptr;
    const ClassInfo* cursor = receiver_class;
    std::set<const ClassInfo*> visited;
    while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
      for (const Member& member : cursor->declaration->members) {
        if (member.kind == MemberKind::kEventDefinition && member.name == block.name) {
          owner = cursor;
          event_definition = &member;
          break;
        }
      }
      if (event_definition != nullptr)
        break;
      if (cursor->declaration->base_name.empty())
        break;
      cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
    }
    if (owner == nullptr || event_definition == nullptr)
      continue;

    const bool pinyin_mode = IsPinyinNaming(config_.friendly_name);
    const std::string definition_part =
        pinyin_mode ? PinyinCamel(event_definition->name) : event_definition->name;
    const std::string listener_name = definition_part + "Listener";
    const std::string listener = names_.ClassName(*owner) + "." + listener_name;
    const std::string object_name =
        renames_.count(block.event_object) != 0 ? renames_[block.event_object] : block.event_object;
    const std::string handler =
        object_name + "$" + (pinyin_mode ? PinyinCamel(block.name) : block.name);
    std::string parameters;
    std::string argument_names;
    for (const Parameter& parameter : event_definition->parameters) {
      const std::string parameter_name = names_.LocalName(parameter.name);
      if (!parameters.empty())
        parameters += ", ";
      parameters +=
          (parameter.type_name.empty() ? "Object" : types_.MapType(parameter.type_name, owner)) +
          " " + parameter_name;
      if (!argument_names.empty())
        argument_names += ", ";
      argument_names += parameter_name;
    }

    Indent(indent, body);
    body->append(object_name + ".set" + listener_name + "(new " + listener + "() {\n");
    Indent(indent + 1, body);
    body->append("@Override\n");
    Indent(indent + 1, body);
    body->append("public void dispatch(" + parameters + ") {\n");
    Indent(indent + kAnonymousClassBodyOffset, body);
    body->append(handler + "(" + argument_names + ");\n");
    Indent(indent + 1, body);
    body->append("}\n");
    Indent(indent, body);
    body->append("});\n");

    for (const Annotation& annotation : event_definition->annotations) {
      if (annotation.name != "属性需求")
        continue;
      const std::string property = annotation.TextArgument(kArgumentParameter);
      if (property.empty())
        continue;
      Indent(indent, body);
      const std::string receiver_name = renames_.count(block.event_object) != 0
                                            ? renames_[block.event_object]
                                            : block.event_object;
      std::string property_name = property;
      const std::string receiver_type = MemberTypeOf(klass, block.event_object);
      if (const ClassInfo* receiver_class = symbols_.Resolve(receiver_type, &klass)) {
        const ClassInfo* owner = nullptr;
        if (const Member* property_member =
                FindMemberInChain(symbols_, *receiver_class, property, &owner)) {
          property_name = names_.MemberName(*property_member);
        }
      }
      body->append(receiver_name + "." + property_name + "(true);\n");
    }
    emitted = true;
  }
  return emitted;
}

void JavaEmitter::EmitIf(const ClassInfo& klass, const Stmt& statement, int indent,
                         bool inline_prefix, EmittedText* out) {
  ConstantValue constant;
  if (statement.expr != nullptr && FoldConstant(*statement.expr, &constant) &&
      constant.kind == ConstantValue::Kind::kBoolean) {
    if (klass.declaration != nullptr) {
      const std::string message = constant.boolean ? "如果语句条件永远为真，已去除其否则部分"
                                                   : "如果语句条件永远为假，已将其去除";
      diagnostics_.Warning(klass.declaration->file, ExpressionStart(*statement.expr), message,
                           "code.constant-if");
    }
    const std::vector<StmtPtr>& emitted = constant.boolean ? statement.body : statement.else_body;
    for (const StmtPtr& child : emitted) {
      EmitStatement(klass, *child, indent, out);
    }
    return;
  }
  if (!inline_prefix)
    Indent(indent, out);
  out->append("if (");
  out->append(statement.expr != nullptr ? EmitExpression(klass, *statement.expr) : "false");
  out->append(") {\n");
  for (const StmtPtr& child : statement.body) {
    EmitStatement(klass, *child, indent + 1, out);
  }

  if (statement.else_body.empty()) {
    out->Mark(current_file_, static_cast<int>(statement.position.line) + 1);
    Indent(indent, out);
    out->append("}\n");
    return;
  }

  if (statement.else_body.size() == 1 && statement.else_body[0] != nullptr &&
      statement.else_body[0]->kind == StmtKind::kIf) {
    out->Mark(current_file_, static_cast<int>(statement.position.line) + 1);
    Indent(indent, out);
    out->append("} else {");
    EmitIf(klass, *statement.else_body[0], indent, true, out);
    if (statement.end_line > 0)
      out->Mark(current_file_, statement.end_line);
    Indent(indent, out);
    out->append("}\n");
    return;
  }

  out->Mark(current_file_, static_cast<int>(statement.position.line) + 1);
  Indent(indent, out);
  out->append("} else {\n");
  for (const StmtPtr& child : statement.else_body) {
    EmitStatement(klass, *child, indent + 1, out);
  }
  if (statement.end_line > 0)
    out->Mark(current_file_, statement.end_line);
  Indent(indent, out);
  out->append("}\n");
}
void JavaEmitter::EmitStatement(const ClassInfo& klass, const Stmt& statement, int indent,
                                EmittedText* out) {
  if (statement.position.line > 0) {
    out->Mark(current_file_, static_cast<int>(statement.position.line) + 1);
  }
  switch (statement.kind) {
    case StmtKind::kExpression: {
      if (statement.expr == nullptr)
        return;
      if (statement.expr->kind == ExprKind::kExchange) {
        EmitSwap(klass, *statement.expr, indent, out);
        return;
      }
      std::string inlined;
      LineRef inline_ref;
      if (TryInlineCode(klass, *statement.expr, &inlined, &inline_ref)) {
        std::vector<std::string> lines = NormalizeCodeBlock(inlined, indent, inline_code_indent_);
        bool separate_semicolon = false;
        const std::string trimmed = Trim(inlined);
        if (!trimmed.empty() && trimmed.back() != ';' && trimmed.back() != '}') {
          if (!lines.empty() && lines.back().find("//") == std::string::npos) {
            lines.back() += ";";
          } else {
            separate_semicolon = true;
          }
        }
        int code_offset = 0;
        for (const std::string& line : lines) {
          if (inline_ref.line > 0) {
            out->Mark(inline_ref.file, inline_ref.line + code_offset);
          }
          ++code_offset;
          out->append(line);
          out->append("\n");
        }
        if (separate_semicolon) {
          if (inline_ref.line > 0)
            out->Mark(inline_ref.file, inline_ref.line + code_offset);
          Indent(indent, out);
          out->append(";\n");
        }
        return;
      }
      Indent(indent, out);
      out->append(EmitExpression(klass, *statement.expr));
      out->append(";\n");
      return;
    }
    case StmtKind::kCode: {
      current_code_line_ = statement.code_line;
      const std::string resolved = SubstituteCode(klass, statement.code_text, current_parameters_);
      current_code_line_ = 0;
      int code_offset = 0;
      for (const std::string& line : NormalizeCodeBlock(resolved, indent, statement.code_indent)) {
        out->Mark(current_file_, (statement.code_first_line > 0 ? statement.code_first_line
                                                                : statement.code_line) +
                                     code_offset);
        ++code_offset;
        out->append(line);
        out->append("\n");
      }
      return;
    }
    case StmtKind::kReturn: {
      Indent(indent, out);
      out->append("return");
      if (statement.expr != nullptr) {
        out->push_back(' ');
        out->append(EmitConverted(klass, current_return_type_, *statement.expr));
      }
      out->append(";\n");
      return;
    }
    case StmtKind::kVariable: {
      if (!IsLocalDeclarationUsed(statement, referenced_locals_)) {
        if (klass.declaration != nullptr) {
          diagnostics_.Warning(klass.declaration->file, statement.position,
                               "局部变量\"" + statement.name + "\"未被使用，已将其去除",
                               "lint.unused-local");
        }
        return;
      }
      if (statement.expr != nullptr && !IsLocalDeclarationUsed(statement, referenced_locals_)) {
        return;
      }
      if (!statement.type_name.empty())
        local_types_[statement.name] = statement.type_name;
      const std::string local_name =
          statement.name.empty() ? statement.name : names_.LocalName(statement.name);
      if (!statement.name.empty()) {
        renames_[statement.name] = local_name;
        current_parameters_[statement.name] = local_name;
      }
      Indent(indent, out);
      const std::string type =
          statement.type_name.empty() ? "Object" : JavaTypeOf(klass, statement.type_name);
      out->append(type + " " + local_name);
      const TypeShape shape = ParseTypeShape(statement.type_name);
      if (statement.expr != nullptr) {
        out->append(" = " + InitializerExpression(klass, statement.type_name, *statement.expr));
      } else if (!statement.code_text.empty()) {
        out->append(" = " + Trim(statement.code_text));
      } else if (shape.dimensions > 0 && !shape.capacities.empty()) {
        std::string array = "new " + types_.MapType(shape.base, &klass);
        for (const std::string& capacity : shape.capacities) {
          array += "[" + capacity + "]";
        }
        out->append(" = " + array);
      } else if (!statement.type_name.empty() &&
                 statement.type_name.find('?') == std::string::npos) {
        const ClassInfo* local_class = symbols_.Resolve(statement.type_name, &klass);
        if (local_class != nullptr && !local_class->has_java_alias && !local_class->forbid_new) {
          out->append(" = new " + type + "()");
        }
      }
      out->append(";\n");
      return;
    }
    case StmtKind::kIf: {
      EmitIf(klass, statement, indent, false, out);
      return;
    }
    case StmtKind::kLoopCount: {
      const std::string counter =
          names_.GeneratedName(kRandomNamePrefix, ++generated_name_counter_);
      const std::string begin =
          statement.loop_begin != nullptr ? EmitExpression(klass, *statement.loop_begin) : "0";
      const std::string end =
          statement.loop_end != nullptr ? EmitExpression(klass, *statement.loop_end) : "0";
      const std::string step =
          statement.loop_step != nullptr ? EmitExpression(klass, *statement.loop_step) : "1";
      Indent(indent, out);
      out->append("for (int " + counter + " = " + begin + ";" + counter + " < " + end + ";" +
                  counter + "+=" + step + ") {\n");
      const auto previous = renames_.find(statement.loop_variable);
      const bool had_previous = previous != renames_.end();
      const std::string saved = had_previous ? previous->second : std::string();
      renames_[statement.loop_variable] = counter;
      for (const StmtPtr& child : statement.body) {
        EmitStatement(klass, *child, indent + 1, out);
      }
      if (had_previous)
        renames_[statement.loop_variable] = saved;
      else
        renames_.erase(statement.loop_variable);
      if (statement.end_line > 0)
        out->Mark(current_file_, statement.end_line);
      Indent(indent, out);
      out->append("}\n");
      return;
    }
    case StmtKind::kLoopWhile: {
      Indent(indent, out);
      out->append("while (");
      out->append(statement.expr != nullptr ? EmitExpression(klass, *statement.expr) : "false");
      out->append(") {\n");
      for (const StmtPtr& child : statement.body) {
        EmitStatement(klass, *child, indent + 1, out);
      }
      if (statement.end_line > 0)
        out->Mark(current_file_, statement.end_line);
      Indent(indent, out);
      out->append("}\n");
      return;
    }
    case StmtKind::kLoopForeach: {
      if (statement.expr == nullptr)
        return;
      std::string collection_type;
      if (statement.expr->kind == ExprKind::kIdentifier) {
        const auto local = local_types_.find(statement.expr->text);
        if (local != local_types_.end())
          collection_type = local->second;
        if (collection_type.empty())
          collection_type = MemberTypeOf(klass, statement.expr->text);
      }
      const TypeShape shape = ParseTypeShape(collection_type);
      std::string element_type = "Object";
      if (shape.dimensions > 0) {
        element_type = types_.MapType(shape.base, &klass);
      } else {
        const std::size_t open = collection_type.find('<');
        const std::size_t close = collection_type.rfind('>');
        if (open != std::string::npos && close != std::string::npos && close > open) {
          const std::string arguments = collection_type.substr(open + 1, close - open - 1);
          std::string first = arguments;
          const std::size_t comma = arguments.find(',');
          if (comma != std::string::npos)
            first = arguments.substr(0, comma);
          element_type = types_.MapType(first, &klass);
        }
      }
      const std::string iterator =
          names_.GeneratedName(kRandomNamePrefix, ++generated_name_counter_);
      Indent(indent, out);
      out->append("for (" + element_type + " " + iterator + " : " +
                  EmitExpression(klass, *statement.expr) + ") {\n");
      const auto previous = renames_.find(statement.loop_key_name);
      const bool had_previous = previous != renames_.end();
      const std::string saved = had_previous ? previous->second : std::string();
      if (!statement.loop_key_name.empty())
        renames_[statement.loop_key_name] = iterator;
      for (const StmtPtr& child : statement.body) {
        EmitStatement(klass, *child, indent + 1, out);
      }
      if (had_previous)
        renames_[statement.loop_key_name] = saved;
      else if (!statement.loop_key_name.empty())
        renames_.erase(statement.loop_key_name);
      if (statement.end_line > 0)
        out->Mark(current_file_, statement.end_line);
      Indent(indent, out);
      out->append("}\n");
      return;
    }
    case StmtKind::kSubscribe: {
      if (EmitSubscriptions(klass, indent, out))
        subscriptions_emitted_ = true;
      return;
    }
    case StmtKind::kSwitch: {
      Indent(indent, out);
      out->append("switch (");
      out->append(statement.expr != nullptr ? EmitExpression(klass, *statement.expr) : "0");
      out->append(") {\n");
      for (std::size_t case_index = 0; case_index < statement.cases.size(); ++case_index) {
        const SwitchCase& switch_case = statement.cases[case_index];
        for (const ExprPtr& value : switch_case.values) {
          if (switch_case.position.line > 0) {
            out->Mark(current_file_, static_cast<int>(switch_case.position.line) + 1);
          }
          Indent(indent, out);
          out->append("case ");
          out->append(value != nullptr ? EmitExpression(klass, *value) : "0");
          out->append(":\n");
        }
        for (const StmtPtr& child : switch_case.body) {
          EmitStatement(klass, *child, indent + 1, out);
        }
        const bool has_next_case = case_index + 1 < statement.cases.size();
        if (has_next_case) {
          const SwitchCase& next_case = statement.cases[case_index + 1];
          if (next_case.position.line > 0) {
            out->Mark(current_file_, static_cast<int>(next_case.position.line) + 1);
          }
        } else if (statement.else_position.line > 0) {
          out->Mark(current_file_, static_cast<int>(statement.else_position.line) + 1);
        } else if (statement.end_line > 0) {
          out->Mark(current_file_, statement.end_line);
        }
        Indent(indent + 1, out);
        out->append("break;\n");
      }
      if (!statement.else_body.empty()) {
        if (statement.else_position.line > 0) {
          out->Mark(current_file_, static_cast<int>(statement.else_position.line) + 1);
        }
        Indent(indent, out);
        out->append("default:\n");
        for (const StmtPtr& child : statement.else_body) {
          EmitStatement(klass, *child, indent + 1, out);
        }
      }
      if (statement.end_line > 0)
        out->Mark(current_file_, statement.end_line);
      Indent(indent, out);
      out->append("}\n");
      return;
    }
    case StmtKind::kBreak:
      Indent(indent, out);
      out->append("break;\n");
      return;
    case StmtKind::kContinue:
      Indent(indent, out);
      out->append("continue;\n");
      return;
    case StmtKind::kEventBlock: {
      if (statement.event_object.empty() || statement.event_name.empty())
        return;
      std::string type_name;
      const auto local = local_types_.find(statement.event_object);
      if (local != local_types_.end())
        type_name = local->second;
      if (type_name.empty())
        type_name = MemberTypeOf(klass, statement.event_object);
      const ClassInfo* receiver_class = symbols_.Resolve(type_name, &klass);
      if (receiver_class == nullptr)
        return;
      const ClassInfo* owner = nullptr;
      const Member* definition =
          FindMemberInChain(symbols_, *receiver_class, statement.event_name, &owner);
      if (definition == nullptr || owner == nullptr)
        return;

      const bool pinyin_mode = IsPinyinNaming(config_.friendly_name);
      const std::string event_part = pinyin_mode ? PinyinCamel(definition->name) : definition->name;
      const std::string listener_name = event_part + "Listener";
      const std::string listener = names_.ClassName(*owner) + "." + listener_name;
      std::string return_type = "void";
      if (!statement.event_type.empty())
        return_type = JavaTypeOf(klass, statement.event_type);
      else if (!definition->type_name.empty()) {
        return_type = types_.MapType(definition->type_name, owner);
      }
      std::string parameters;
      std::string argument_names;
      const std::vector<Parameter>& params =
          statement.event_parameters.empty() ? definition->parameters : statement.event_parameters;
      for (const Parameter& parameter : params) {
        const std::string parameter_name = names_.LocalName(parameter.name);
        if (!parameters.empty())
          parameters += ", ";
        parameters +=
            (parameter.type_name.empty() ? "Object" : JavaTypeOf(klass, parameter.type_name)) +
            " " + parameter_name;
        if (!argument_names.empty())
          argument_names += ", ";
        argument_names += parameter_name;
      }

      const auto saved_locals = local_types_;
      const auto saved_parameters = current_parameter_types_;
      std::map<std::string, std::string> saved_renames;
      for (const Parameter& parameter : params) {
        if (!parameter.type_name.empty())
          local_types_[parameter.name] = parameter.type_name;
        current_parameter_types_.push_back(parameter);
        const std::string translated = names_.LocalName(parameter.name);
        current_parameters_[parameter.name] = translated;
        const auto existing_rename = renames_.find(parameter.name);
        if (existing_rename != renames_.end())
          saved_renames[parameter.name] = existing_rename->second;
        renames_[parameter.name] = translated;
      }
      Indent(indent, out);
      const std::string statement_receiver = renames_.count(statement.event_object) != 0
                                                 ? renames_[statement.event_object]
                                                 : names_.LocalName(statement.event_object);
      out->append(statement_receiver + ".set" + listener_name + "(new " + listener + "() {\n");
      Indent(indent + 1, out);
      out->append("@Override\n");
      Indent(indent + 1, out);
      out->append("public " + return_type + " dispatch(" + parameters + ") {\n");
      for (const StmtPtr& child : statement.body) {
        EmitStatement(klass, *child, indent + kAnonymousClassBodyOffset, out);
      }
      if (statement.end_line > 0)
        out->Mark(current_file_, statement.end_line);
      Indent(indent + 1, out);
      out->append("}\n");
      Indent(indent, out);
      out->append("});\n");
      local_types_ = saved_locals;
      current_parameter_types_ = saved_parameters;
      for (const Parameter& parameter : params) {
        const auto saved = saved_renames.find(parameter.name);
        if (saved != saved_renames.end()) {
          renames_[parameter.name] = saved->second;
        } else {
          renames_.erase(parameter.name);
        }
      }
      return;
    }
    case StmtKind::kBlock:
      for (const StmtPtr& child : statement.body) {
        EmitStatement(klass, *child, indent, out);
      }
      return;
  }
}
bool JavaEmitter::IsComponentClass(const ClassInfo& klass) const {
  const ClassInfo* cursor = &klass;
  std::set<const ClassInfo*> visited;
  while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
    const std::string simple = cursor->qualified_name.substr(cursor->qualified_name.rfind('.') + 1);
    if (simple == kWindowComponentClass || simple == kVisualComponentClass ||
        simple == kLayoutComponentClass)
      return true;
    if (cursor->declaration->base_name.empty())
      break;
    cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
  }
  return false;
}
bool JavaEmitter::IsComponentContainerClass(const ClassInfo& klass) const {
  const ClassInfo* cursor = &klass;
  std::set<const ClassInfo*> visited;
  while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second) {
    const std::string simple = cursor->qualified_name.substr(cursor->qualified_name.rfind('.') + 1);
    if (simple == kComponentContainerClass)
      return true;
    if (cursor->declaration->base_name.empty())
      break;
    cursor = symbols_.Resolve(cursor->declaration->base_name, cursor);
  }
  return false;
}
bool JavaEmitter::HasDeclaredConstructor(const ClassInfo& klass) const {
  if (klass.declaration == nullptr)
    return false;
  const std::string marker = "#cls<" + klass.declaration->name + ">";
  const auto inspect = [&](const std::string& code) {
    return code.find(marker + "(") != std::string::npos ||
           code.find(marker + " (") != std::string::npos;
  };
  for (const Member& member : klass.declaration->members) {
    if (!member.code_text.empty() && inspect(member.code_text))
      return true;
    for (const StmtPtr& statement : member.body) {
      if (statement != nullptr && statement->kind == StmtKind::kCode &&
          inspect(statement->code_text)) {
        return true;
      }
    }
  }
  return false;
}

void JavaEmitter::ReportUnsupported(const ClassInfo& klass, const SourcePosition& position,
                                    const std::string& message, const std::string& key) {
  const std::string dedup_key =
      key + "|" + message + "|" + (klass.declaration != nullptr ? klass.declaration->file : "");
  if (!reported_.insert(dedup_key).second)
    return;
  const std::string file = klass.declaration != nullptr ? klass.declaration->file : std::string();
  diagnostics_.Warning(file, position, message, key);
}

void JavaEmitter::ReportUnresolvedType(const ClassInfo& klass, const std::string& type_name) {
  ReportUnsupported(klass,
                    klass.declaration != nullptr ? klass.declaration->position : SourcePosition(),
                    "暂不支持的未解析类型: \"" + type_name + "\"", "codegen.type.unresolved");
}

}
