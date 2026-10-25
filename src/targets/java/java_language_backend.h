#ifndef TIECODEC_TARGETS_JAVA_JAVA_LANGUAGE_BACKEND_H
#define TIECODEC_TARGETS_JAVA_JAVA_LANGUAGE_BACKEND_H

#include <memory>

#include "java_platform_extension.h"
#include "target.h"

namespace tiecodec {

class JavaLanguageBackend : public LanguageBackend {
public:
  explicit JavaLanguageBackend(std::shared_ptr<JavaPlatformExtension> extension);

  TargetKey Key() const override;
  GeneratedProgram Emit(const GenerationInput& input, Diagnostics& diagnostics) override;

private:
  std::shared_ptr<JavaPlatformExtension> extension_;
};

}

#endif
