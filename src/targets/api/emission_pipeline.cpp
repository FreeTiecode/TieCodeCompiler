#include "emission_pipeline.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace tiecodec {

bool LanguageFactoryRegistry::RegisterNaming(const std::string& language, NamingFactory factory) {
  if (language.empty() || !factory)
    return false;
  return namings_.emplace(language, std::move(factory)).second;
}

bool LanguageFactoryRegistry::RegisterType(const std::string& language, TypeFactory factory) {
  if (language.empty() || !factory)
    return false;
  return types_.emplace(language, std::move(factory)).second;
}

const NamingFactory* LanguageFactoryRegistry::FindNaming(const std::string& language) const {
  const auto found = namings_.find(language);
  return found == namings_.end() ? nullptr : &found->second;
}

const TypeFactory* LanguageFactoryRegistry::FindType(const std::string& language) const {
  const auto found = types_.find(language);
  return found == types_.end() ? nullptr : &found->second;
}

LanguageFactoryRegistry& BuiltinLanguageFactories() {
  static LanguageFactoryRegistry registry;
  return registry;
}

std::unique_ptr<NamingScheme> CreateLanguageNamingScheme(const std::string& language,
                                                         const LanguageNamingRequest& request) {
  const NamingFactory* factory = BuiltinLanguageFactories().FindNaming(language);
  if (factory == nullptr || !*factory)
    return nullptr;
  return (*factory)(request);
}

std::unique_ptr<TypeMapper> CreateLanguageTypeMapper(const std::string& language,
                                                     const LanguageTypeRequest& request) {
  const TypeFactory* factory = BuiltinLanguageFactories().FindType(language);
  if (factory == nullptr || !*factory)
    return nullptr;
  return (*factory)(request);
}

}

namespace tiecodec {

void EmitProgram(const GenerationInput& input, SourceEmitter& emitter, NamingScheme& names,
                 const EmissionFileNames& extensions, Diagnostics& diagnostics,
                 GeneratedProgram* program) {
  if (program == nullptr)
    return;

  std::set<std::string> written_names;
  const auto add_source = [&](const std::string& directory, const std::string& file_stem,
                              const std::string& extension, std::string content,
                              std::vector<LineRef> line_refs, const ClassInfo* klass) -> bool {
    const std::string relative = directory + file_stem + "." + extension;
    if (!written_names.insert(relative).second) {
      diagnostics.ErrorGlobal("输出类名冲突: " + file_stem, "output.duplicate-class");
      return false;
    }
    GeneratedSource source;
    source.file_name = file_stem + "." + extension;
    source.directory = directory;
    source.content = std::move(content);
    source.line_refs = std::move(line_refs);
    source.klass = klass;
    program->sources.push_back(std::move(source));
    if (klass != nullptr)
      program->qualified_names[klass] = names.Package(*klass) + "." + file_stem;
    return true;
  };

  const auto emit_class = [&](const ClassInfo* klass, std::set<const ClassInfo*>* emitted) -> bool {
    if (klass == nullptr || klass->declaration == nullptr || klass->forbid_output)
      return true;
    if (emitted != nullptr)
      emitted->insert(klass);
    if (klass->HasExternalType(extensions.language) &&
        !klass->declaration->generic_parameters.empty()) {
      program->qualified_names[klass] = names.Package(*klass) + "." + names.ClassName(*klass);
      return true;
    }
    ClassOutput output = emitter.EmitClass(*klass);
    const std::vector<AuxiliaryClass> auxiliaries = emitter.TakeAuxiliaryClasses();
    bool header_written = false;
    for (const AuxiliaryClass& auxiliary : auxiliaries) {
      if (!auxiliary.is_header)
        continue;
      const std::string extension =
          auxiliary.extension.empty() ? extensions.primary_extension : auxiliary.extension;
      if (header_written) {
        if (!add_source(auxiliary.directory.empty() ? std::string() : auxiliary.directory,
                        auxiliary.name, extension, auxiliary.text, auxiliary.line_refs, nullptr))
          return false;
        continue;
      }
      if (!add_source(auxiliary.directory, auxiliary.name, extension, auxiliary.text,
                      auxiliary.line_refs, klass))
        return false;
      header_written = true;
    }
    if (!header_written) {
      const std::string class_name = names.ClassName(*klass);
      if (!add_source(std::string(), class_name, extensions.primary_extension,
                      std::move(output.text), std::move(output.line_refs), klass))
        return false;
    }
    for (const AuxiliaryClass& auxiliary : auxiliaries) {
      if (auxiliary.is_header)
        continue;
      const std::string extension =
          auxiliary.extension.empty() ? extensions.secondary_extension : auxiliary.extension;
      if (!add_source(auxiliary.directory, auxiliary.name, extension, auxiliary.text,
                      auxiliary.line_refs, nullptr))
        return false;
    }
    return true;
  };

  std::set<const ClassInfo*> emitted_classes;
  for (const ClassInfo* klass : input.plan.classes) {
    if (klass == nullptr || klass->declaration == nullptr || klass->forbid_output)
      continue;
    if (!emit_class(klass, &emitted_classes))
      return;
  }

  bool generic_progress = true;
  int generic_loop_guard = 0;
  while (generic_progress) {
    if (++generic_loop_guard > kMaxGenericExpansionRounds)
      break;
    generic_progress = false;
    for (const GenericInstantiationSink::Request& instantiation :
         emitter.TakePendingGenericInstantiations()) {
      const std::string file_stem =
          instantiation.file_stem.empty() ? instantiation.output_name : instantiation.file_stem;
      const std::string directory =
          instantiation.directory.empty() ? extensions.directory : instantiation.directory;
      const std::string relative = directory + file_stem + "." + extensions.primary_extension;
      if (!written_names.insert(relative).second) {
        diagnostics.ErrorGlobal("输出类名冲突: " + file_stem, "output.duplicate-class");
        return;
      }
      ClassOutput generated = emitter.EmitGenericInstantiation(instantiation);
      GeneratedSource source;
      source.file_name = file_stem + "." + extensions.primary_extension;
      source.directory = directory;
      source.content = std::move(generated.text);
      source.line_refs = std::move(generated.line_refs);
      program->sources.push_back(std::move(source));
      generic_progress = true;
    }
    for (const ClassInfo* klass : emitter.TakeGenericNeededClasses()) {
      if (klass == nullptr || klass->declaration == nullptr)
        continue;
      if (!emitted_classes.insert(klass).second)
        continue;
      if (!emit_class(klass, &emitted_classes))
        return;
      const ClassInfo* cursor = klass;
      std::set<const ClassInfo*> visited;
      while (cursor != nullptr && cursor->declaration != nullptr && visited.insert(cursor).second &&
             !cursor->declaration->base_name.empty()) {
        cursor = input.symbols.Resolve(cursor->declaration->base_name, cursor);
        if (cursor == nullptr)
          break;
        if (!emitted_classes.insert(cursor).second)
          continue;
        if (!emit_class(cursor, &emitted_classes))
          return;
      }
      generic_progress = true;
    }
  }

  std::set<std::string> mapping_seen;
  const auto add_mapping = [&](const std::string& source_name, const std::string& output_name) {
    if (source_name.empty() || output_name.empty())
      return;
    if (!mapping_seen.insert(source_name + "\n" + output_name).second)
      return;
    program->mapping_names.emplace_back(source_name, output_name);
  };
  for (const ClassInfo* klass : input.plan.classes) {
    if (klass == nullptr || klass->declaration == nullptr || klass->forbid_output)
      continue;
    if (!klass->package_name.empty())
      add_mapping(klass->package_name, klass->package_name);
    const std::string& qualified = klass->qualified_name;
    const std::size_t dot = qualified.rfind('.');
    add_mapping(dot == std::string::npos ? qualified : qualified.substr(dot + 1),
                names.ClassName(*klass));
    for (const Member& member : klass->declaration->members) {
      if (input.plan.members.count(&member) == 0)
        continue;
      add_mapping(member.name, names.MemberName(member));
    }
  }

  if (input.entry != nullptr) {
    program->entry_class_name = names.ClassName(*input.entry);
    program->entry_qualified_name = names.Package(*input.entry) + "." + program->entry_class_name;
  }
}

}
