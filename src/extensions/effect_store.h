#ifndef TIECODEC_EXTENSIONS_EFFECT_STORE_H
#define TIECODEC_EXTENSIONS_EFFECT_STORE_H

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tiecodec {

class IEffectTable {
public:
  virtual ~IEffectTable() = default;
};

template <typename T>
class EffectTable : public IEffectTable {
public:
  using Entry = std::pair<const void*, T>;

  void Set(const void* node, T value) {
    for (Entry& entry : entries_) {
      if (entry.first == node) {
        entry.second = std::move(value);
        return;
      }
    }
    entries_.emplace_back(node, std::move(value));
  }

  const T* Get(const void* node) const {
    for (const Entry& entry : entries_) {
      if (entry.first == node)
        return &entry.second;
    }
    return nullptr;
  }

  T* GetMutable(const void* node) {
    for (Entry& entry : entries_) {
      if (entry.first == node)
        return &entry.second;
    }
    return nullptr;
  }

  T& Ensure(const void* node) {
    if (T* existing = GetMutable(node))
      return *existing;
    entries_.emplace_back(node, T{});
    return entries_.back().second;
  }

  const std::vector<Entry>& entries() const {
    return entries_;
  }

private:
  std::vector<Entry> entries_;
};

class EffectStore {
public:
  template <typename T>
  EffectTable<T>& Table() {
    const std::type_index key(typeid(T));
    const auto found = tables_.find(key);
    if (found != tables_.end())
      return *static_cast<EffectTable<T>*>(found->second.get());
    auto table = std::make_unique<EffectTable<T>>();
    EffectTable<T>* raw = table.get();
    tables_.emplace(key, std::move(table));
    return *raw;
  }

  template <typename T>
  const EffectTable<T>* FindTable() const {
    const auto found = tables_.find(std::type_index(typeid(T)));
    if (found == tables_.end())
      return nullptr;
    return dynamic_cast<const EffectTable<T>*>(found->second.get());
  }

  template <typename T>
  void Set(const void* node, T value) {
    Table<T>().Set(node, std::move(value));
  }

  template <typename T>
  const T* Get(const void* node) const {
    const EffectTable<T>* table = FindTable<T>();
    return table == nullptr ? nullptr : table->Get(node);
  }

private:
  std::unordered_map<std::type_index, std::unique_ptr<IEffectTable>> tables_;
};

}

#endif
