#ifndef TIECODEC_ANALYSIS_SYMBOLS_H
#define TIECODEC_ANALYSIS_SYMBOLS_H

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "constants.h"
#include "diagnostics.h"
#include "semantic_model.h"

namespace tiecodec {

struct ClassInfo {
  const ClassDecl* declaration = nullptr;
  std::unordered_map<std::string, const Member*> member_by_name;
  std::string qualified_name;
  std::string package_name;

  std::map<std::string, std::string> external_types;
  std::string output_name;
  bool global_base = false;
  bool forbid_new = false;
  bool is_global_class = false;
  bool forbid_output = false;
  bool hidden = false;
  bool deprecated = false;
  bool disable_pointer = false;
  std::string external_base;

  std::string output_package;
  int end_ref_line = 0;

  std::string BaseQualifiedName() const;
  bool HasExternalType(const std::string& language) const;
  std::string ExternalType(const std::string& language) const;
};

struct GlobalMemberRef {
  const ClassInfo* owner = nullptr;
  const Member* member = nullptr;
};

class SymbolTable {
public:
  bool AddUnit(const Unit& unit, const SemanticModel& semantics, Diagnostics* diagnostics);

  const ClassInfo* Resolve(const std::string& type_name, const ClassInfo* context_class) const;
  const ClassInfo* Find(const std::string& qualified_name) const;

  std::string OutputPackageFor(const std::string& package_name) const;

  const std::vector<std::string>& class_names() const {
    return class_names_;
  }
  const std::map<std::string, ClassInfo>& classes() const {
    return classes_;
  }
  const std::vector<const ClassInfo*>& global_classes() const {
    return global_classes_;
  }
  const std::vector<GlobalMemberRef>* GlobalMembers(const std::string& name) const;
  std::size_t class_count() const {
    return class_names_.size();
  }

private:
  void BuildGlobalMemberIndex() const;

  std::map<std::string, ClassInfo> classes_;
  std::vector<std::string> class_names_;
  std::map<std::string, const ClassInfo*> simple_name_index_;
  std::map<std::string, std::map<std::string, const ClassInfo*>> package_classes_;
  std::vector<const ClassInfo*> global_classes_;
  mutable std::unordered_map<std::string, std::vector<GlobalMemberRef>> global_member_index_;
  mutable bool global_member_index_ready_ = false;
  int global_base_count_ = 0;
  std::map<std::string, std::string> package_output_names_;
};

}

#endif
