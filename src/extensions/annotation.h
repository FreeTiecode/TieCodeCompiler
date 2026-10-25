#ifndef TIECODEC_EXTENSIONS_ANNOTATION_H
#define TIECODEC_EXTENSIONS_ANNOTATION_H

#include <string>
#include <utility>
#include <vector>

#include "constants.h"
#include "target_key.h"

namespace tiecodec {

struct Unit;
struct ClassDecl;
struct Member;
struct ClassInfo;
struct Annotation;
struct ValidationContext;
struct DeclareContext;
struct ApplyContext;

enum class AnnotationLevel : unsigned {
  kUnit = 1,
  kClass = 2,
  kVariable = 4,
  kMethod = 8,
};

inline constexpr unsigned kLevelUnit = static_cast<unsigned>(AnnotationLevel::kUnit);
inline constexpr unsigned kLevelClass = static_cast<unsigned>(AnnotationLevel::kClass);
inline constexpr unsigned kLevelVariable = static_cast<unsigned>(AnnotationLevel::kVariable);
inline constexpr unsigned kLevelMethod = static_cast<unsigned>(AnnotationLevel::kMethod);
inline constexpr unsigned kAnyMember = kLevelClass | kLevelVariable | kLevelMethod;
inline constexpr unsigned kAllLevels = kLevelUnit | kAnyMember;

enum class AnnotationPhase : unsigned {
  kCondition = 1,
  kDeclare = 2,
  kApply = 4,
};

enum class ParamKind {
  kAny,
  kText,
  kBool,
  kTypeName,
  kMemberName,
  kCondition,
  kTly,
};

enum class FileSearchKind {
  kNone,
  kSource,
  kResource,
  kLibrary,
  kAsset,
};

struct ParamRule {
  std::string name;
  ParamKind kind = ParamKind::kAny;
  bool optional = false;
  bool required = false;
};

struct AnnotationMetadata {
  std::string name;
  unsigned levels = 0;
  TargetFilter target;
  std::vector<ParamRule> positional;
  std::vector<ParamRule> named;
  unsigned phases = 0;
  int priority = 0;
  FileSearchKind file_kind = FileSearchKind::kNone;
};

inline ParamRule TextParam(std::string name = kArgumentParameter) {
  return {std::move(name), ParamKind::kText, false, false};
}

inline ParamRule AnyParam(std::string name = kArgumentParameter) {
  return {std::move(name), ParamKind::kAny, false, false};
}

inline ParamRule BoolParam(std::string name = kArgumentParameter) {
  return {std::move(name), ParamKind::kBool, true, false};
}

inline ParamRule RequiredNamedParam(std::string name, ParamKind kind = ParamKind::kText) {
  return {std::move(name), kind, false, true};
}

inline unsigned Phases(AnnotationPhase phase) {
  return static_cast<unsigned>(phase);
}

inline unsigned operator|(AnnotationPhase left, AnnotationPhase right) {
  return static_cast<unsigned>(left) | static_cast<unsigned>(right);
}

inline AnnotationMetadata MakeMetadata(std::string name, unsigned levels, unsigned phases,
                                       std::vector<ParamRule> positional = {},
                                       std::vector<ParamRule> named = {},
                                       TargetFilter target = AnyTarget(), int priority = 0,
                                       FileSearchKind file_kind = FileSearchKind::kNone) {
  AnnotationMetadata metadata;
  metadata.name = std::move(name);
  metadata.levels = levels;
  metadata.phases = phases;
  metadata.positional = std::move(positional);
  metadata.named = std::move(named);
  metadata.target = std::move(target);
  metadata.priority = priority;
  metadata.file_kind = file_kind;
  return metadata;
}

struct DeclRef {
  const Unit* unit = nullptr;
  const ClassDecl* klass = nullptr;
  const Member* member = nullptr;
  const ClassInfo* owner = nullptr;
  AnnotationLevel level = AnnotationLevel::kUnit;
};

class AnnotationProcessor {
public:
  virtual ~AnnotationProcessor() = default;

  virtual AnnotationMetadata Metadata() const = 0;

  virtual void Validate(const Annotation& annotation, const DeclRef& declaration,
                        ValidationContext& context) const {
    (void)annotation;
    (void)declaration;
    (void)context;
  }

  virtual void Declare(const Annotation& annotation, const DeclRef& declaration,
                       DeclareContext& context) const {
    (void)annotation;
    (void)declaration;
    (void)context;
  }

  virtual void Apply(const Annotation& annotation, const DeclRef& declaration,
                     ApplyContext& context) const {
    (void)annotation;
    (void)declaration;
    (void)context;
  }
};

template <typename Derived>
class AnnotationBase : public AnnotationProcessor {
public:
  AnnotationMetadata Metadata() const final {
    return Derived::CreateMetadata();
  }
};

}

#endif
