#ifndef TIECODEC_IR_SEMANTIC_MODEL_H
#define TIECODEC_IR_SEMANTIC_MODEL_H

#include <map>
#include <string>
#include <unordered_map>

#include "ast.h"

namespace tiecodec {

struct UnitFacts {
  std::string output_package;
};

struct ClassFacts {
  std::string output_name;
  bool force_output = false;
  bool global_base = false;
  bool global_class = false;
  bool forbid_output = false;
  bool forbid_new = false;
  bool forbid_inherit = false;
  bool deprecated = false;
  bool disable_pointer = false;
  bool hidden = false;
  bool value_type = false;
  std::string external_base;
  std::string value_output_rule;
  std::string match_file_pattern;
  std::map<std::string, std::string> external_types;
};

struct MemberFacts {
  std::string output_name;
  bool force_use = false;
  bool is_virtual = false;
  bool hidden = false;
  std::string property_demand;
  bool async = false;
  bool async_independent_thread = false;
};

class SemanticModel {
public:
  UnitFacts& UnitFactsFor(const Unit* unit);
  ClassFacts& ClassFactsFor(const ClassDecl* klass);
  MemberFacts& MemberFactsFor(const Member* member);

  const UnitFacts* FindUnit(const Unit* unit) const;
  const ClassFacts* FindClass(const ClassDecl* klass) const;
  const MemberFacts* FindMember(const Member* member) const;

  const std::string* ExternalType(const ClassDecl* klass, const std::string& language) const;

  const std::unordered_map<const ClassDecl*, ClassFacts>& classes() const {
    return classes_;
  }

  const std::unordered_map<const Member*, MemberFacts>& members() const {
    return members_;
  }

private:
  std::unordered_map<const Unit*, UnitFacts> units_;
  std::unordered_map<const ClassDecl*, ClassFacts> classes_;
  std::unordered_map<const Member*, MemberFacts> members_;
};

}

#endif
