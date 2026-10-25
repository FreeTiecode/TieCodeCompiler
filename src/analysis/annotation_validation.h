#ifndef TIECODEC_ANALYSIS_ANNOTATION_VALIDATION_H
#define TIECODEC_ANALYSIS_ANNOTATION_VALIDATION_H

#include <set>
#include <vector>

#include "annotation_context.h"
#include "annotation_index.h"
#include "annotation_registry.h"
#include "ast.h"
#include "diagnostics.h"
#include "semantic_model.h"
#include "target_key.h"

namespace tiecodec {

void ValidateAnnotations(const Unit& unit, const AnnotationSymbolIndex& index,
                         const AnnotationRegistry& registry, const TargetKey& target,
                         const ValidationEnvironment& environment, Diagnostics* diagnostics,
                         std::set<const Annotation*>* invalid_annotations);

void ValidateAnnotationFiles(const Unit& unit, const AnnotationRegistry& registry,
                             const TargetKey& target, const ValidationEnvironment& environment,
                             Diagnostics* diagnostics,
                             const std::set<const Annotation*>* invalid_annotations);

void ValidateForbiddenCreation(const std::vector<Unit>& units, const SemanticModel& semantics,
                               Diagnostics* diagnostics);

}

#endif
