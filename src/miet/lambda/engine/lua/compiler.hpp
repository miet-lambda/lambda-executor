#pragma once

#include <miet/lambda/engine/base/compiler.hpp>

#include <miet/lambda/engine/lua/script.hpp>

#include <LuaCpp/Registry/LuaCompiler.hpp>

namespace miet::lambda::engine::lua {
class Compiler final : public CompilerBase {
 public:
  [[nodiscard]] ScriptPtr Compile(std::string_view code) const override;

 private:
  /** @todo Mark "CompileString" method as "const" and remove "mutable" */
  mutable LuaCpp::Registry::LuaCompiler impl_;
};
}  // namespace miet::lambda::engine::lua
