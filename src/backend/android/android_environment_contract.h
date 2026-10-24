#ifndef TIECODEC_BACKEND_ANDROID_ENVIRONMENT_CONTRACT_H
#define TIECODEC_BACKEND_ANDROID_ENVIRONMENT_CONTRACT_H

#include "environment_contract.h"

namespace tiecodec {

class AndroidEnvironmentContract : public EnvironmentContract {
public:
  const std::vector<RequiredType>& RequiredTypes() const override;
  const std::vector<RequiredMember>& RequiredMembers() const override;
};

}

#endif
