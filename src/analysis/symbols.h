#ifndef TIECODEC_ANALYSIS_SYMBOLS_H
#define TIECODEC_ANALYSIS_SYMBOLS_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include "ast.h"
#include "constants.h"
#include "diagnostics.h"

namespace tiecodec {

struct ClassInfo {
  const ClassDecl* declaration = nullptr;
  std::string qualified_name;
  std::string package_name;

  bool has_java_alias = false;
  std::string java_alias;
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
};

class SymbolTable {
public:
  bool AddUnit(const Unit& unit, Diagnostics* diagnostics);

  const ClassInfo* Resolve(const std::string& type_name, const ClassInfo* context_class) const;
  const ClassInfo* Find(const std::string& qualified_name) const;

  std::string OutputPackageFor(const std::string& package_name) const;

  const std::vector<std::string>& class_names() const {
    return class_names_;
  }
  const std::map<std::string, ClassInfo>& classes() const {
    return classes_;
  }

private:
  std::map<std::string, ClassInfo> classes_;
  std::vector<std::string> class_names_;
  int global_base_count_ = 0;
  std::map<std::string, std::string> package_output_names_;
};

}

#endif
