#ifndef TIECODEC_TARGETS_API_EMISSION_PIPELINE_H
#define TIECODEC_TARGETS_API_EMISSION_PIPELINE_H

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "diagnostics.h"
#include "emitted_text.h"
#include "naming.h"
#include "target.h"
#include "target_config.h"
#include "type_mapper.h"

namespace tiecodec {

class GenericInstantiationSink {
public:
  struct Request {
    const ClassInfo* base = nullptr;
    std::vector<std::string> arguments;
    std::string output_name;
    std::string file_stem;
    std::string directory;
  };

  virtual ~GenericInstantiationSink() = default;

  virtual std::vector<Request> TakePendingGenericInstantiations() = 0;
  virtual ClassOutput EmitGenericInstantiation(const Request& request) = 0;
  virtual std::vector<const ClassInfo*> TakeGenericNeededClasses() = 0;
};

class SourceEmitter : public GenericInstantiationSink {
public:
  virtual ~SourceEmitter() = default;

  virtual void SetPlan(
      const std::set<const Member*>& used_members,
      const std::vector<std::pair<const ClassInfo*, const Member*>>& virtual_overrides,
      const std::vector<const ClassInfo*>& output_classes) = 0;

  virtual bool IsGenericClass(const ClassInfo& klass) const = 0;
  virtual ClassOutput EmitClass(const ClassInfo& klass) = 0;
  virtual std::vector<AuxiliaryClass> TakeAuxiliaryClasses() = 0;
  virtual void RunAnalysisLints() = 0;
};

struct LanguageNamingRequest {
  const SymbolTable& symbols;
  const TargetConfig& config;
  const SemanticModel& semantics;
};

struct LanguageTypeRequest {
  const SymbolTable& symbols;
  const NamingScheme* names;
};

using NamingFactory = std::function<std::unique_ptr<NamingScheme>(const LanguageNamingRequest&)>;

using TypeFactory = std::function<std::unique_ptr<TypeMapper>(const LanguageTypeRequest&)>;

class LanguageFactoryRegistry {
public:
  bool RegisterNaming(const std::string& language, NamingFactory factory);
  bool RegisterType(const std::string& language, TypeFactory factory);

  const NamingFactory* FindNaming(const std::string& language) const;
  const TypeFactory* FindType(const std::string& language) const;

private:
  std::map<std::string, NamingFactory> namings_;
  std::map<std::string, TypeFactory> types_;
};

LanguageFactoryRegistry& BuiltinLanguageFactories();

std::unique_ptr<NamingScheme> CreateLanguageNamingScheme(const std::string& language,
                                                         const LanguageNamingRequest& request);

std::unique_ptr<TypeMapper> CreateLanguageTypeMapper(const std::string& language,
                                                     const LanguageTypeRequest& request);

struct EmissionFileNames {
  std::string primary_extension;
  std::string secondary_extension;
  std::string directory;
  std::string language;
};

void EmitProgram(const GenerationInput& input, SourceEmitter& emitter, NamingScheme& names,
                 const EmissionFileNames& extensions, Diagnostics& diagnostics,
                 GeneratedProgram* program);

}

#endif
