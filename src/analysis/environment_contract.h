#ifndef TIECODEC_ANALYSIS_ENVIRONMENT_CONTRACT_H
#define TIECODEC_ANALYSIS_ENVIRONMENT_CONTRACT_H

#include <string>
#include <vector>

#include "diagnostics.h"
#include "symbols.h"

namespace tiecodec {

struct RequiredType {
  std::string name;
};

struct RequiredMember {
  std::string class_name;
  std::string member_name;
};

class EnvironmentContract {
public:
  virtual ~EnvironmentContract() = default;
  virtual const std::vector<RequiredType>& RequiredTypes() const = 0;
  virtual const std::vector<RequiredMember>& RequiredMembers() const = 0;
};

void ValidateEnvironment(const EnvironmentContract& contract, const SymbolTable& symbols,
                         Diagnostics* diagnostics);

}

#endif
