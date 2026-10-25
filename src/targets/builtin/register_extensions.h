#ifndef TIECODEC_TARGETS_BUILTIN_REGISTER_EXTENSIONS_H
#define TIECODEC_TARGETS_BUILTIN_REGISTER_EXTENSIONS_H

#include <string>

#include "extension_registry.h"

namespace tiecodec {

bool RegisterBuiltinExtensions(ExtensionRegistry& registry, std::string* error);

}

#endif
