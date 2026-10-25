#ifndef TIECODEC_IR_ANNOTATION_INDEX_H
#define TIECODEC_IR_ANNOTATION_INDEX_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include "ast.h"

namespace tiecodec {

struct AnnotationSymbolIndex {
  std::set<std::string> class_names;
  std::set<std::string> builtin_types;
  std::set<std::string> constant_names;
  std::set<std::string> member_names;
  std::map<std::string, std::set<std::string>> class_constants;
  std::map<std::string, std::set<std::string>> class_members;
  std::map<std::string, std::string> class_base;
  std::map<std::string, std::string> class_package;

  bool ResolveSymbol(const std::string& name) const;
  bool ResolveTypeName(const std::string& name) const;
  bool ResolveMemberName(const std::string& name, const std::string& owner_class = "") const;
  bool ClassChainContains(const std::string& type_name, const std::string& target) const;
  std::string QualifiedNameOf(const std::string& class_name) const;
};

void BuildAnnotationSymbolIndex(const std::vector<Unit>& units, AnnotationSymbolIndex* index);

}

#endif
