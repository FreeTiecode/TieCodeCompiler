#ifndef TIECODEC_EXTENSIONS_REQUIREMENT_H
#define TIECODEC_EXTENSIONS_REQUIREMENT_H

#include <string>

#include "plan_hints.h"
#include "target_key.h"

namespace tiecodec {

struct SymbolTable;
struct SemanticModel;
struct Diagnostics;

enum class RequirementKind {
  kType,
  kMember,
};

struct RequirementMetadata {
  std::string id;
  RequirementKind kind = RequirementKind::kType;
  std::string type_name;
  std::string owner_class;
  std::string member_name;
  TargetFilter target;
  bool seed_use = true;
  bool virtual_hook = false;
};

class PlatformRequirement {
public:
  virtual ~PlatformRequirement() = default;

  virtual RequirementMetadata Metadata() const = 0;

  virtual void Validate(const SymbolTable& symbols, Diagnostics& diagnostics) const;

  virtual void Apply(const SymbolTable& symbols, const SemanticModel& semantics,
                     PlanHints& plan) const;
};

class DeclarativeRequirement final : public PlatformRequirement {
public:
  explicit DeclarativeRequirement(RequirementMetadata metadata);

  RequirementMetadata Metadata() const override;

private:
  RequirementMetadata metadata_;
};

}

#endif
