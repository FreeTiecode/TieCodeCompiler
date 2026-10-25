#ifndef TIECODEC_TARGETS_JAVA_JAVA_COMPONENT_H
#define TIECODEC_TARGETS_JAVA_JAVA_COMPONENT_H

#include <string>
#include <vector>

namespace tiecodec {

struct Member;

struct ComponentProperty {
  std::string name;
  std::string value;
  bool parent = false;
};

struct ComponentField {
  const Member* member = nullptr;
  std::string parent;
  bool is_root = false;
  std::vector<ComponentProperty> properties;
};

}

#endif
