#pragma once

#include <miet/lambda/base/key-value-storage.hpp>

#include <LuaCpp/LuaCpp.hpp>

namespace miet::lambda::lua {
class Storage final : public LuaCpp::LuaMetaObject {
 public:
  explicit Storage(std::int64_t projectId,
                   KeyValueStoragePtr kvStorage) noexcept;

  std::shared_ptr<LuaCpp::Engine::LuaType> getValue(std::string& key) override;

 private:
  KeyValueStoragePtr kvStorage_ = nullptr;
  std::shared_ptr<LuaCpp::Engine::LuaType> storeMethod_ = nullptr;
  std::shared_ptr<LuaCpp::Engine::LuaType> getMethod_ = nullptr;
};
}  // namespace miet::lambda::lua
