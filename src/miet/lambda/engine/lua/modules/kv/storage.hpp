#pragma once

#include <miet/lambda/project/base/storage.hpp>

#include <LuaCpp/LuaCpp.hpp>

namespace miet::lambda::engine::lua::modules::kv {
class Storage final : public LuaCpp::LuaMetaObject {
 public:
  explicit Storage(ProjectIdType projectId,
                   project::StoragePtr kvStorage) noexcept;

  std::shared_ptr<LuaCpp::Engine::LuaType> getValue(std::string& key) override;

 private:
  project::StoragePtr kvStorage_ = nullptr;
  std::shared_ptr<LuaCpp::Engine::LuaType> storeMethod_ = nullptr;
  std::shared_ptr<LuaCpp::Engine::LuaType> getMethod_ = nullptr;
};
}  // namespace miet::lambda::engine::lua::modules::kv
