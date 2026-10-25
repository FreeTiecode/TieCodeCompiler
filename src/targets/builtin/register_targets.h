#ifndef TIECODEC_TARGETS_BUILTIN_REGISTER_TARGETS_H
#define TIECODEC_TARGETS_BUILTIN_REGISTER_TARGETS_H

#include <string>

#include "target_registry.h"

namespace tiecodec {

bool RegisterBuiltinTargets(TargetRegistry& registry, std::string* error);

}

#endif
