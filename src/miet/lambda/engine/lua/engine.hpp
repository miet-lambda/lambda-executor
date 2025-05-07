#pragma once

#include <miet/lambda/engine/base/engine.hpp>

#include <miet/lambda/engine/fwd.hpp>

namespace miet::lambda::engine::lua {
class ScriptEngine final : public ScriptEngineBase {
 public:
  ScriptEngine(ExecutionControlParams execCtlParams, ExtraParams extraParams);

  [[nodiscard]] CompilerPtr CreateCompiler() const override;
  [[nodiscard]] ExecutorPtr CreateExecutor() const override;

 private:
  ExecutionControlParams execCtlParams_;
  ExtraParams extraParams_;
};
}  // namespace miet::lambda::engine::lua
