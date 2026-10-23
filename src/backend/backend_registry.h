#ifndef TIECODEC_BACKEND_BACKEND_REGISTRY_H
#define TIECODEC_BACKEND_BACKEND_REGISTRY_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "backend.h"
#include "symbols.h"

namespace tiecodec {

class BackendRegistry {
public:
  using Factory = std::function<std::unique_ptr<Backend>(const SymbolTable&)>;

  void Register(const std::string& platform, Factory factory);
  std::unique_ptr<Backend> Create(const std::string& platform, const SymbolTable& symbols) const;
  std::vector<std::string> platforms() const;

  static BackendRegistry& Default();

private:
  std::map<std::string, Factory> factories_;
};

std::unique_ptr<Backend> CreateBackend(const std::string& platform, const SymbolTable& symbols);

}  // namespace tiecodec

#endif
