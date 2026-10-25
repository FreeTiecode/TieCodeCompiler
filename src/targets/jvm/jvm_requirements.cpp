#include "jvm_requirements.h"

#include <memory>
#include <string>

#include "jvm_constants.h"
#include "requirement.h"
#include "requirement_registry.h"
#include "target_key.h"

namespace tiecodec {

void RegisterJvmRequirements(RequirementRegistry& registry) {
  RequirementMetadata console;
  console.id = "jvm.type.控制台";
  console.kind = RequirementKind::kType;
  console.type_name = "控制台";
  console.target = PlatformTarget(kJvmPlatformName);
  registry.Register(std::make_unique<DeclarativeRequirement>(console));

  RequirementMetadata output;
  output.id = "jvm.member.控制台.输出";
  output.kind = RequirementKind::kMember;
  output.owner_class = "控制台";
  output.member_name = "输出";
  output.target = PlatformTarget(kJvmPlatformName);
  registry.Register(std::make_unique<DeclarativeRequirement>(output));
}

}
