#pragma once

#include <miet/lambda/engine/base/compiler.hpp>

#include <miet/lambda/engine/stub/script.hpp>

namespace miet::lambda::engine::stub {
class Compiler final : public CompilerBase {
 public:
  ScriptPtr Compile([[maybe_unused]] std::string_view code) const override {
    const auto script = std::make_shared<Script>();
    script->SetSourceCode(code);
    return script;
  }
};
}  // namespace miet::lambda::engine::stub
