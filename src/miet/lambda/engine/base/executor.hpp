#pragma once

#include <miet/lambda/engine/base/script.hpp>

#include <miet/lambda/execution-context.hpp>

namespace miet::lambda::engine {
class ExecutorBase {
 public:
  virtual ~ExecutorBase() = default;
  virtual void Execute(ScriptPtr script, ExecutionContextRef context) = 0;
};

using ExecutorPtr = std::shared_ptr<ExecutorBase>;
}  // namespace miet::lambda::engine
