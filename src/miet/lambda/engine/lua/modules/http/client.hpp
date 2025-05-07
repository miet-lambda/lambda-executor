#pragma once

#include <miet/lambda/http/base/client.hpp>

#include <LuaCpp/LuaCpp.hpp>

namespace miet::lambda::engine::lua::modules::http {
class Client final : public LuaCpp::LuaMetaObject {
 public:
  explicit Client(miet::lambda::http::ClientPtr httpClient) noexcept;

  std::shared_ptr<LuaCpp::Engine::LuaType> getValue(std::string& key) override;

 private:
  miet::lambda::http::ClientPtr httpClient_ = nullptr;
  std::shared_ptr<LuaCpp::Engine::LuaType> getClientMethod_ = nullptr;
};
}  // namespace miet::lambda::engine::lua::modules::http
