#include "symbols.h"

#include "constants.h"

#include <algorithm>

namespace tiecodec {
namespace {

std::string BaseTypeName(const std::string& type_name) {
  std::string result;
  int depth = 0;
  for (char c : type_name) {
    if (c == '<') {
      ++depth;
      continue;
    }
    if (c == '>') {
      --depth;
      continue;
    }
    if (depth > 0)
      continue;
    if (c == '[')
      break;
    if (c == '?')
      break;
    result.push_back(c);
  }
  return result;
}

std::string SimpleName(const std::string& qualified_name) {
  const size_t dot = qualified_name.rfind('.');
  return dot == std::string::npos ? qualified_name : qualified_name.substr(dot + 1);
}

bool IsValidOutputIdentifier(const std::string& name) {
  if (name.empty())
    return false;
  bool first = true;
  for (char raw : name) {
    const unsigned char c = static_cast<unsigned char>(raw);
    const bool ascii_alpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    const bool ascii_digit = c >= '0' && c <= '9';
    if (first) {
      if (!(ascii_alpha || c == '_' || c == '$' || c >= 0x80))
        return false;
      first = false;
      continue;
    }
    if (!(ascii_alpha || ascii_digit || c == '_' || c == '$' || c >= 0x80))
      return false;
  }
  return true;
}

}

std::string ClassInfo::BaseQualifiedName() const {
  if (declaration == nullptr || declaration->base_name.empty())
    return {};
  return declaration->base_name;
}

bool SymbolTable::AddUnit(const Unit& unit, Diagnostics* diagnostics) {
  std::string unit_output_package;
  if (const Annotation* package_name = unit.FindAnnotation(annotation::kOutputName)) {
    const std::string value = package_name->TextArgument(kArgumentParameter);
    if (value.find('.') != std::string::npos) {
      unit_output_package = value;
      if (!unit.package_name.empty())
        package_output_names_[unit.package_name] = value;
    }
  }
  for (const ClassDecl& class_decl : unit.classes) {
    ClassInfo info;
    info.declaration = &class_decl;
    info.package_name = unit.package_name;
    info.output_package = unit_output_package;
    info.qualified_name =
        unit.package_name.empty() ? class_decl.name : unit.package_name + "." + class_decl.name;

    if (const Annotation* alias = class_decl.FindAnnotation("指代类")) {
      info.has_java_alias = true;
      info.java_alias = alias->TextArgument(kArgumentParameter);
    }
    if (const Annotation* output = class_decl.FindAnnotation(annotation::kOutputName)) {
      const std::string output_name = output->TextArgument(kArgumentParameter);
      if (!IsValidOutputIdentifier(output_name)) {
        if (diagnostics != nullptr) {
          diagnostics->Error(unit.file, class_decl.position,
                             "类输出名不是合法的 Java 标识符: " + output_name, "enter.output-name");
        }
      } else {
        info.output_name = output_name;
      }
    }
    info.global_base = class_decl.HasAnnotation("全局基础类");
    info.forbid_new = class_decl.HasAnnotation(annotation::kNoInstantiation);
    info.is_global_class = class_decl.HasAnnotation("全局类");
    info.forbid_output = class_decl.HasAnnotation("禁止输出");
    info.hidden = class_decl.HasAnnotation(annotation::kHidden);
    info.deprecated = class_decl.HasAnnotation("废弃使用");
    info.disable_pointer = class_decl.HasAnnotation("禁用指针");
    if (const Annotation* external = class_decl.FindAnnotation("外部基础类")) {
      info.external_base = external->TextArgument(kArgumentParameter);
      if (!class_decl.base_name.empty() && diagnostics != nullptr) {
        diagnostics->Error(unit.file, class_decl.base_position, "类指定外部基础类之后不能继承父类",
                           "parse.external-base.inherit");
      }
    }
    if (info.global_base && diagnostics != nullptr) {
      ++global_base_count_;
      if (global_base_count_ > 1) {
        diagnostics->Error(unit.file, class_decl.keyword_position, "全局基础类只能定义一个",
                           "parse.global-base.duplicate");
      }
    }

    const auto inserted = classes_.emplace(info.qualified_name, std::move(info));
    if (!inserted.second) {
      if (diagnostics != nullptr) {
        diagnostics->ErrorGlobal("类重复定义: " + inserted.first->first, "enter.duplicated-class");
      }
      continue;
    }
    ClassInfo& stored = inserted.first->second;
    for (const Member& member : class_decl.members)
      stored.member_by_name.emplace(member.name, &member);
    class_names_.push_back(stored.qualified_name);
    const auto index_entry = simple_name_index_.emplace(SimpleName(stored.qualified_name), &stored);
    if (!index_entry.second)
      index_entry.first->second = nullptr;
    if (stored.is_global_class)
      global_classes_.push_back(&stored);
  }
  global_member_index_ready_ = false;
  for (std::size_t i = 0; i < unit.classes.size(); ++i) {
    const ClassDecl& class_decl = unit.classes[i];
    const std::string qualified =
        unit.package_name.empty() ? class_decl.name : unit.package_name + "." + class_decl.name;
    const auto it = classes_.find(qualified);
    if (it == classes_.end())
      continue;
    it->second.end_ref_line =
        (i + 1 < unit.classes.size()) ? unit.classes[i + 1].declaration_line : unit.line_count;
  }
  return true;
}

std::string SymbolTable::OutputPackageFor(const std::string& package_name) const {
  const auto it = package_output_names_.find(package_name);
  return it == package_output_names_.end() ? std::string() : it->second;
}

const ClassInfo* SymbolTable::Find(const std::string& qualified_name) const {
  const auto it = classes_.find(qualified_name);
  return it == classes_.end() ? nullptr : &it->second;
}

const ClassInfo* SymbolTable::Resolve(const std::string& type_name,
                                      const ClassInfo* context_class) const {
  const std::string base = BaseTypeName(type_name);
  if (base.empty())
    return nullptr;

  if (const ClassInfo* exact = Find(base))
    return exact;

  if (context_class != nullptr && !context_class->package_name.empty()) {
    if (const ClassInfo* same_package = Find(context_class->package_name + "." + base)) {
      return same_package;
    }
  }

  const auto simple = simple_name_index_.find(base);
  return simple == simple_name_index_.end() ? nullptr : simple->second;
}

void SymbolTable::BuildGlobalMemberIndex() const {
  global_member_index_.clear();
  for (const std::string& qualified_name : class_names_) {
    const auto it = classes_.find(qualified_name);
    if (it == classes_.end() || !it->second.is_global_class || it->second.declaration == nullptr)
      continue;
    for (const Member& member : it->second.declaration->members)
      global_member_index_[member.name].push_back(GlobalMemberRef{&it->second, &member});
  }
  global_member_index_ready_ = true;
}

const std::vector<GlobalMemberRef>* SymbolTable::GlobalMembers(const std::string& name) const {
  if (!global_member_index_ready_)
    BuildGlobalMemberIndex();
  const auto it = global_member_index_.find(name);
  return it == global_member_index_.end() ? nullptr : &it->second;
}

}
