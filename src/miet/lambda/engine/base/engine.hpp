#pragma once

#include <miet/lambda/engine/base/compiler.hpp>
#include <miet/lambda/engine/base/executor.hpp>

namespace miet::lambda::engine {
class ScriptEngineBase {
 public:
  virtual ~ScriptEngineBase() = default;

  [[nodiscard]] virtual CompilerPtr CreateCompiler() const = 0;
  [[nodiscard]] virtual ExecutorPtr CreateExecutor() const = 0;
};

using ScriptEnginePtr = std::shared_ptr<ScriptEngineBase>;
}  // namespace miet::lambda::engine
