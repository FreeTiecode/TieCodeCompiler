#ifndef TIECODEC_TARGETS_CXX_CXX_LANGUAGE_BACKEND_H
#define TIECODEC_TARGETS_CXX_CXX_LANGUAGE_BACKEND_H

#include <memory>

#include "target.h"

namespace tiecodec {

class CxxLanguageBackend : public LanguageBackend {
public:
  CxxLanguageBackend();

  TargetKey Key() const override;
  GeneratedProgram Emit(const GenerationInput& input, Diagnostics& diagnostics) override;
};

}

#endif
