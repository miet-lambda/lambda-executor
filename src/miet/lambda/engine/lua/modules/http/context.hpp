#pragma once

#include <miet/lambda/execution-context.hpp>

#include <LuaCpp/LuaCpp.hpp>

namespace miet::lambda::engine::lua::modules::http {
using LuaEnvRef = LuaCpp::LuaEnvironment&;

class Context final : public LuaCpp::LuaMetaObject {
 public:
  Context(LuaEnvRef luaEnv, ExecutionContextRef executionContext);
  ~Context();

  void PopulateResponse();

 private:
  LuaEnvRef luaEnv_;
  ExecutionContextRef executionContext_;
};
}  // namespace miet::lambda::engine::lua::modules::http
