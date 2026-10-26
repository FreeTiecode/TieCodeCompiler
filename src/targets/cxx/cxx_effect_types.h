#ifndef TIECODEC_TARGETS_CXX_CXX_EFFECT_TYPES_H
#define TIECODEC_TARGETS_CXX_CXX_EFFECT_TYPES_H

#include <string>
#include <vector>

namespace tiecodec {

struct CxxIncludeList {
  std::vector<std::string> includes;
};

struct CxxExternalSource {
  std::vector<std::string> files;
};

struct CxxModifiers {
  bool hidden = false;
  bool is_static = false;
  bool is_external = false;
  bool inline_code = false;
  bool has_initializer = false;
  std::string initializer;
};

struct CxxExternalDependency {
  std::vector<std::string> libraries;
};

}

#endif
