#ifndef TIECODEC_ANALYSIS_ANNOTATION_RULES_H
#define TIECODEC_ANALYSIS_ANNOTATION_RULES_H

#include <string>
#include <vector>

#include "ast.h"
#include "diagnostics.h"

namespace tiecodec {

/// Validates annotation names, target levels and arguments for the Android
/// platform the way the reference compiler reports them.
void ValidateAnnotations(const Unit& unit, Diagnostics* diagnostics);

/// Validates existence of files referenced by class-level file annotations.
void ValidateAnnotationFiles(const Unit& unit, const std::vector<std::string>& source_prefixes,
                             const std::vector<std::string>& res_prefixes,
                             const std::vector<std::string>& lib_prefixes,
                             const std::vector<std::string>& assets_prefixes,
                             Diagnostics* diagnostics);

}  // namespace tiecodec

#endif
