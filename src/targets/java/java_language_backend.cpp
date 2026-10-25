#include "java_language_backend.h"

#include "java_language_constants.h"

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "constants.h"
#include "java_emitter.h"
#include "java_names.h"
#include "java_platform_extension.h"
#include "java_type_mapper.h"
#include "options.h"

namespace tiecodec {

JavaLanguageBackend::JavaLanguageBackend(std::shared_ptr<JavaPlatformExtension> extension)
    : extension_(std::move(extension)) {}

TargetKey JavaLanguageBackend::Key() const {
  return TargetKey{"", kJavaLanguageName};
}

GeneratedProgram JavaLanguageBackend::Emit(const GenerationInput& input, Diagnostics& diagnostics) {
  GeneratedProgram result;
  if (extension_ == nullptr)
    return result;

  TargetConfig config = input.config;

  JavaNames names(input.symbols, config, input.semantics);
  const std::string stable_names_path = config.Option(option::kStableNames);
  if (!stable_names_path.empty())
    names.LoadStableNames(stable_names_path);
  JavaTypeMapper types(input.symbols, &names);
  const JavaPlatformServices services{*extension_, *extension_, *extension_};
  JavaEmitter emitter(input.symbols, config, input.semantics, input.effects, input.units, names,
                      types, services, diagnostics, extension_->Resources());
  emitter.SetPlan(input.plan.members, input.plan.virtual_overrides, input.plan.classes);
  emitter.RunAnalysisLints();

  std::set<std::string> written_names;
  const auto add_source = [&](const std::string& file_stem, std::string content,
                              std::vector<LineRef> line_refs, const ClassInfo* klass) -> bool {
    if (!written_names.insert(file_stem).second) {
      diagnostics.ErrorGlobal("输出类名冲突: " + file_stem, "output.duplicate-class");
      return false;
    }
    GeneratedSource source;
    source.file_name = file_stem + ".java";
    source.content = std::move(content);
    source.line_refs = std::move(line_refs);
    source.klass = klass;
    result.sources.push_back(std::move(source));
    if (klass != nullptr)
      result.qualified_names[klass] = names.Package(*klass) + "." + file_stem;
    return true;
  };

  const auto emit_class = [&](const ClassInfo* klass, std::set<const ClassInfo*>* emitted) -> bool {
    if (klass == nullptr || klass->declaration == nullptr || klass->forbid_output)
      return true;
    if (emitted != nullptr)
      emitted->insert(klass);
    if (klass->HasExternalType("java") && !klass->declaration->generic_parameters.empty()) {
      result.qualified_names[klass] = names.Package(*klass) + "." + names.ClassName(*klass);
      return true;
    }
    const std::string class_name = names.ClassName(*klass);
    ClassOutput output = emitter.EmitClass(*klass);
    if (!add_source(class_name, std::move(output.text), std::move(output.line_refs), klass))
      return false;
    for (const auto& auxiliary : emitter.TakeAuxiliaryClasses()) {
      if (!add_source(auxiliary.first, auxiliary.second, {}, nullptr))
        return false;
    }
    return true;
  };

  std::set<const ClassInfo*> emitted_classes;
  for (const ClassInfo* klass : input.plan.classes) {
    if (klass == nullptr || klass->declaration == nullptr || klass->forbid_output)
      continue;
    if (!emit_class(klass, &emitted_classes))
      return result;
  }

  bool generic_progress = true;
  int generic_loop_guard = 0;
  while (generic_progress) {
    if (++generic_loop_guard > kMaxGenericExpansionRounds)
      break;
    generic_progress = false;
    for (const JavaEmitter::GenericInstantiation& instantiation :
         emitter.TakePendingGenericInstantiations()) {
      if (!written_names.insert(instantiation.output_name).second) {
        diagnostics.ErrorGlobal("输出类名冲突: " + instantiation.output_name,
                                "output.duplicate-class");
        return result;
      }
      GeneratedSource source;
      source.file_name = instantiation.output_name + ".java";
      source.content = emitter.EmitGenericInstantiation(instantiation);
      result.sources.push_back(std::move(source));
      generic_progress = true;
    }
    for (const ClassInfo* klass : emitter.TakeGenericNeededClasses()) {
      if (klass == nullptr || klass->declaration == nullptr)
        continue;
      if (!emitted_classes.insert(klass).second)
        continue;
      if (!emit_class(klass, &emitted_classes))
        return result;
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
          return result;
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
    result.mapping_names.emplace_back(source_name, output_name);
  };
  for (const ClassInfo* klass : input.plan.classes) {
    if (klass == nullptr || klass->declaration == nullptr)
      continue;
    if (!klass->package_name.empty())
      add_mapping(klass->package_name, klass->package_name);
    const std::string simple = klass->qualified_name.substr(klass->qualified_name.rfind('.') + 1);
    add_mapping(simple, names.ClassName(*klass));
    for (const Member& member : klass->declaration->members) {
      if (input.plan.members.count(&member) == 0)
        continue;
      std::string output_name = names.MemberName(member);
      if (member.kind == MemberKind::kEventBlock && !member.event_object.empty() &&
          input.symbols.Resolve(member.event_object, klass) == nullptr) {
        output_name = member.event_object + "$" + output_name;
      }
      add_mapping(member.name, output_name);
    }
  }

  if (input.entry != nullptr) {
    result.entry_class_name = names.ClassName(*input.entry);
    result.entry_qualified_name = names.Package(*input.entry) + "." + result.entry_class_name;
  }

  const std::string emit_names_path = config.Option(option::kEmitNames);
  if (!emit_names_path.empty())
    names.SaveNames(emit_names_path);
  return result;
}

}
