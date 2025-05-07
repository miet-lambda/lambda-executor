#pragma once

#include <miet/lambda/engine/base/script.hpp>

#include <LuaCpp/Registry/LuaCodeSnippet.hpp>

namespace miet::lambda::engine::lua {
class Script final : public ScriptBase {
 public:
  Script() noexcept;

  void SetCodeSnippet(LuaCpp::Registry::LuaCodeSnippet&& snippet);
  LuaCpp::Registry::LuaCodeSnippet& GetCodeSnippet() & noexcept;

 private:
  LuaCpp::Registry::LuaCodeSnippet snippet_;
};
}  // namespace miet::lambda::engine::lua
