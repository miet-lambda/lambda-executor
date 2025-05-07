#pragma once

#include <miet/lambda/engine/base/engine.hpp>

#include <miet/lambda/engine/stub/compiler.hpp>
#include <miet/lambda/engine/stub/executor.hpp>

namespace miet::lambda::engine::stub {
class ScriptEngine final : public ScriptEngineBase {
 public:
  [[nodiscard]] CompilerPtr CreateCompiler() const override {
    return std::make_shared<Compiler>();
  }
  [[nodiscard]] ExecutorPtr CreateExecutor() const override {
    return std::make_shared<Executor>();
  }
};
}  // namespace miet::lambda::engine::stub
