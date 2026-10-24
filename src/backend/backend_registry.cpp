#include "backend_registry.h"

#include "android_backend.h"

namespace tiecodec {

void BackendRegistry::Register(const std::string& platform, Factory factory) {
  factories_[platform] = std::move(factory);
}

std::unique_ptr<Backend> BackendRegistry::Create(const std::string& platform,
                                                 const SymbolTable& symbols) const {
  const auto it = factories_.find(platform);
  if (it == factories_.end())
    return nullptr;
  return it->second(symbols);
}

std::vector<std::string> BackendRegistry::platforms() const {
  std::vector<std::string> names;
  for (const auto& entry : factories_)
    names.push_back(entry.first);
  return names;
}

BackendRegistry& BackendRegistry::Default() {
  static BackendRegistry registry = [] {
    BackendRegistry instance;
    instance.Register("android", [](const SymbolTable& symbols) {
      return std::make_unique<AndroidBackend>(symbols);
    });
    return instance;
  }();
  return registry;
}

std::unique_ptr<Backend> CreateBackend(const std::string& platform, const SymbolTable& symbols) {
  return BackendRegistry::Default().Create(platform, symbols);
}

}
