#ifndef TIECODEC_IR_OUTPUT_PLAN_H
#define TIECODEC_IR_OUTPUT_PLAN_H

#include <set>
#include <utility>
#include <vector>

#include "symbols.h"

namespace tiecodec {

struct OutputPlan {
  std::vector<const ClassInfo*> classes;
  std::set<const Member*> members;

  std::vector<std::pair<const ClassInfo*, const Member*>> virtual_overrides;
};

}

#endif
