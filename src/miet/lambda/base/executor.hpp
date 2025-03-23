#pragma once

#include <miet/lambda/execution-context.hpp>

namespace miet::lambda {
class ExecutorBase {
 public:
  virtual ~ExecutorBase() = default;
  virtual void Execute(std::string_view id, ExecutionContextRef context) = 0;
};

using ExecutorPtr = std::shared_ptr<ExecutorBase>;

}  // namespace miet::lambda
