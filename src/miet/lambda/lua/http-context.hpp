#pragma once

#include <miet/lambda/execution-context.hpp>

#include <LuaCpp/LuaCpp.hpp>

namespace miet::lambda::lua {
using LuaContextRef = userver::utils::SharedRef<LuaCpp::LuaContext>;
using LuaEnvWeakPtr = std::weak_ptr<LuaCpp::LuaEnvironment>;

class HttpContext final : public LuaCpp::LuaMetaObject {
 public:
  HttpContext(LuaEnvWeakPtr luaEnv, ExecutionContextRef executionContext);

  void PopulateResponse();

  std::shared_ptr<LuaCpp::Engine::LuaType> getValue(std::string& key) override;

 private:
  LuaEnvWeakPtr luaEnv_;
  ExecutionContextRef executionContext_;
  std::shared_ptr<LuaCpp::Engine::LuaType> getRequestMethod_;
  std::shared_ptr<LuaCpp::Engine::LuaType> getResponseMethod_;
};
}  // namespace miet::lambda::lua
