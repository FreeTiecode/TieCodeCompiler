#include "annotation_pipeline.h"

#include <functional>
#include <string>

#include "annotation_context.h"
#include "compile_conditions.h"
#include "event_channel.h"

namespace tiecodec {
namespace {

std::string QualifiedClassName(const Unit& unit, const ClassDecl& klass) {
  return unit.package_name.empty() ? klass.name : unit.package_name + "." + klass.name;
}

using Visitor = std::function<void(const Annotation&, const DeclRef&)>;

void ForEachAnnotation(const std::vector<Unit>& units, const SymbolTable* symbols,
                       const Visitor& visitor) {
  for (const Unit& unit : units) {
    const DeclRef unit_declaration{&unit, nullptr, nullptr, nullptr, AnnotationLevel::kUnit};
    for (const Annotation& annotation : unit.annotations)
      visitor(annotation, unit_declaration);
    for (const ClassDecl& klass : unit.classes) {
      const ClassInfo* owner =
          symbols == nullptr ? nullptr : symbols->Find(QualifiedClassName(unit, klass));
      const DeclRef class_declaration{&unit, &klass, nullptr, owner, AnnotationLevel::kClass};
      for (const Annotation& annotation : klass.annotations)
        visitor(annotation, class_declaration);
      for (const Member& member : klass.members) {
        if (member.kind == MemberKind::kEventBlock || member.kind == MemberKind::kCodeBlock)
          continue;
        AnnotationLevel level = AnnotationLevel::kVariable;
        if (member.kind == MemberKind::kMethod || member.kind == MemberKind::kPropertyRead ||
            member.kind == MemberKind::kPropertyWrite ||
            member.kind == MemberKind::kEventDefinition) {
          level = AnnotationLevel::kMethod;
        }
        const DeclRef member_declaration{&unit, &klass, &member, owner, level};
        for (const Annotation& annotation : member.annotations)
          visitor(annotation, member_declaration);
      }
    }
  }
}

struct DeclareEvent {
  const Annotation* annotation = nullptr;
  const DeclRef* declaration = nullptr;
  DeclareContext* context = nullptr;
  const AnnotationProcessor* processor = nullptr;
};

struct ApplyEvent {
  const Annotation* annotation = nullptr;
  const DeclRef* declaration = nullptr;
  ApplyContext* context = nullptr;
  const AnnotationProcessor* processor = nullptr;
};

}

void RunCompileConditions(std::vector<Unit>& units, const std::map<std::string, bool>& defines) {
  for (Unit& unit : units)
    ApplyCompileConditions(&unit, defines);
}

void DeclareAnnotations(const std::vector<Unit>& units, const AnnotationRegistry& registry,
                        const TargetKey& target, const AnnotationSymbolIndex& index,
                        SemanticModel& semantics, Diagnostics& diagnostics) {
  EventChannel<DeclareEvent> channel;
  channel.Subscribe([](DeclareEvent& event) {
    event.processor->Declare(*event.annotation, *event.declaration, *event.context);
  });
  ForEachAnnotation(units, nullptr, [&](const Annotation& annotation, const DeclRef& declaration) {
    const AnnotationProcessor* processor = registry.Find(annotation.name, target);
    if (processor == nullptr ||
        (processor->Metadata().phases & Phases(AnnotationPhase::kDeclare)) == 0)
      return;
    DeclareContext context{target, index, *declaration.unit, semantics, diagnostics};
    DeclareEvent event{&annotation, &declaration, &context, processor};
    channel.Publish(event);
  });
}

void ApplyAnnotations(const std::vector<Unit>& units, const SymbolTable& symbols,
                      const AnnotationRegistry& registry, const TargetKey& target,
                      const SemanticModel& semantics, EffectStore& effects, PlanHints& plan,
                      Diagnostics& diagnostics) {
  EventChannel<ApplyEvent> channel;
  channel.Subscribe([](ApplyEvent& event) {
    event.processor->Apply(*event.annotation, *event.declaration, *event.context);
  });
  ForEachAnnotation(units, &symbols, [&](const Annotation& annotation, const DeclRef& declaration) {
    const AnnotationProcessor* processor = registry.Find(annotation.name, target);
    if (processor == nullptr ||
        (processor->Metadata().phases & Phases(AnnotationPhase::kApply)) == 0)
      return;
    ApplyContext context{target, symbols, semantics, effects, plan, diagnostics};
    ApplyEvent event{&annotation, &declaration, &context, processor};
    channel.Publish(event);
  });
}

}
