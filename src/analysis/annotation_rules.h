#ifndef TIECODEC_ANALYSIS_ANNOTATION_RULES_H
#define TIECODEC_ANALYSIS_ANNOTATION_RULES_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include "ast.h"
#include "diagnostics.h"

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
  std::set<std::string> forbidden_creation;

  bool ResolveSymbol(const std::string& name) const;
  bool ResolveTypeName(const std::string& name) const;
  bool ResolveMemberName(const std::string& name, const std::string& owner_class = "") const;
  bool ClassChainContains(const std::string& type_name, const std::string& target) const;
  std::string QualifiedNameOf(const std::string& class_name) const;
};

void BuildAnnotationSymbolIndex(const std::vector<Unit>& units, AnnotationSymbolIndex* index);

void ValidateAnnotations(const Unit& unit, const AnnotationSymbolIndex& index,
                         Diagnostics* diagnostics,
                         std::set<const Annotation*>* invalid_annotations);

void ValidateAnnotationFiles(const Unit& unit, const std::vector<std::string>& source_prefixes,
                             const std::vector<std::string>& res_prefixes,
                             const std::vector<std::string>& lib_prefixes,
                             const std::vector<std::string>& assets_prefixes,
                             Diagnostics* diagnostics,
                             const std::set<const Annotation*>* invalid_annotations);

void ValidateForbiddenCreation(const Unit& unit, const AnnotationSymbolIndex& index,
                               Diagnostics* diagnostics);

}

#endif
