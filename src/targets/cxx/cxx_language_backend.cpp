#include "cxx_language_backend.h"

#include <memory>
#include <string>

#include "cxx_emitter.h"
#include "cxx_language_constants.h"
#include "cxx_names.h"
#include "cxx_type_mapper.h"
#include "emission_pipeline.h"
#include "options.h"

namespace tiecodec {

CxxLanguageBackend::CxxLanguageBackend() = default;

TargetKey CxxLanguageBackend::Key() const {
  return TargetKey{"", kCxxLanguageName};
}

GeneratedProgram CxxLanguageBackend::Emit(const GenerationInput& input, Diagnostics& diagnostics) {
  GeneratedProgram result;
  TargetConfig config = input.config;

  CxxNames names(input.symbols, config, input.semantics);
  CxxTypeMapper types(input.symbols, &names);
  CxxEmitter emitter(input.symbols, config, input.semantics, input.effects, names, types,
                     diagnostics);
  emitter.SetPlan(input.plan.members, input.plan.virtual_overrides, input.plan.classes);
  emitter.RunAnalysisLints();

  EmissionFileNames extensions;
  extensions.primary_extension = kCxxHeaderExtension;
  extensions.secondary_extension = kCxxSourceExtension;
  extensions.language = kCxxLanguageName;
  extensions.directory = kCxxSourceDirectory;
  EmitProgram(input, emitter, names, extensions, diagnostics, &result);
  return result;
}

}
