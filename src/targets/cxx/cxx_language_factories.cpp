#include "cxx_language_factories.h"

#include <memory>

#include "cxx_language_constants.h"
#include "cxx_names.h"
#include "cxx_type_mapper.h"
#include "emission_pipeline.h"

namespace tiecodec {

bool RegisterCxxLanguageFactories(LanguageFactoryRegistry& registry) {
  const bool naming = registry.RegisterNaming(
      kCxxLanguageName, [](const LanguageNamingRequest& request) -> std::unique_ptr<NamingScheme> {
        return std::make_unique<CxxNames>(request.symbols, request.config, request.semantics);
      });
  const bool type = registry.RegisterType(
      kCxxLanguageName, [](const LanguageTypeRequest& request) -> std::unique_ptr<TypeMapper> {
        return std::make_unique<CxxTypeMapper>(request.symbols, request.names);
      });
  return naming && type;
}

}
