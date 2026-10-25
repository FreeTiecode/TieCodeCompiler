#ifndef TIECODEC_TARGETS_JAVA_JAVA_EFFECT_TYPES_H
#define TIECODEC_TARGETS_JAVA_JAVA_EFFECT_TYPES_H

#include <string>
#include <vector>

namespace tiecodec {

struct JavaModifiers {
  bool hidden = false;
  bool is_static = false;
  std::string prefix_code;
  std::string suffix_code;
};

struct JavaInlineCodeSpec {
  std::string statement_marker;
  std::string next_statement;
  std::string block_type;
};

struct JavaImportList {
  std::vector<std::string> imports;
};

struct JavaDebugSpec {
  bool enabled = true;
};

struct JavaImplicitOperationSpec {
  std::string kind;
};

}

#endif
