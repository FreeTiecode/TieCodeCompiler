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

}

std::string ClassInfo::BaseQualifiedName() const {
  if (declaration == nullptr || declaration->base_name.empty())
    return {};
  return declaration->base_name;
}

bool ClassInfo::HasExternalType(const std::string& language) const {
  return external_types.find(language) != external_types.end();
}

std::string ClassInfo::ExternalType(const std::string& language) const {
  const auto it = external_types.find(language);
  return it == external_types.end() ? std::string() : it->second;
}

bool SymbolTable::AddUnit(const Unit& unit, const SemanticModel& semantics,
                          Diagnostics* diagnostics) {
  const UnitFacts* unit_facts = semantics.FindUnit(&unit);
  const std::string unit_output_package =
      unit_facts == nullptr ? std::string() : unit_facts->output_package;
  if (!unit_output_package.empty() && !unit.package_name.empty())
    package_output_names_[unit.package_name] = unit_output_package;
  for (const ClassDecl& class_decl : unit.classes) {
    ClassInfo info;
    info.declaration = &class_decl;
    info.package_name = unit.package_name;
    info.output_package = unit_output_package;
    info.qualified_name =
        unit.package_name.empty() ? class_decl.name : unit.package_name + "." + class_decl.name;

    if (const ClassFacts* facts = semantics.FindClass(&class_decl)) {
      info.output_name = facts->output_name;
      info.global_base = facts->global_base;
      info.forbid_new = facts->forbid_new;
      info.is_global_class = facts->global_class;
      info.forbid_output = facts->forbid_output;
      info.hidden = facts->hidden;
      info.deprecated = facts->deprecated;
      info.disable_pointer = facts->disable_pointer;
      info.external_base = facts->external_base;
      info.external_types = facts->external_types;
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
    package_classes_[stored.package_name].emplace(class_decl.name, &stored);
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
  std::string stripped;
  const std::string& base = type_name.find_first_of("<[?") == std::string::npos
                                ? type_name
                                : (stripped = BaseTypeName(type_name));
  if (base.empty())
    return nullptr;

  if (const ClassInfo* exact = Find(base))
    return exact;

  if (context_class != nullptr && !context_class->package_name.empty()) {
    const auto package = package_classes_.find(context_class->package_name);
    if (package != package_classes_.end()) {
      const auto same_package = package->second.find(base);
      if (same_package != package->second.end())
        return same_package->second;
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
