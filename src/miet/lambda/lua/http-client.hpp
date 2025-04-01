#pragma once

#include <miet/lambda/base/http-client.hpp>

#include <LuaCpp/LuaCpp.hpp>

namespace miet::lambda::lua {
class HttpClient final : public LuaCpp::LuaMetaObject {
 public:
  explicit HttpClient(HttpClientPtr httpClient) noexcept;

  std::shared_ptr<LuaCpp::Engine::LuaType> getValue(std::string& key) override;

 private:
  HttpClientPtr httpClient_ = nullptr;
  std::shared_ptr<LuaCpp::Engine::LuaType> getClientMethod_ = nullptr;
};
}  // namespace miet::lambda::lua
