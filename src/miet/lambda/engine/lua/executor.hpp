#pragma once

#include <miet/lambda/engine/base/executor.hpp>

#include <miet/lambda/engine/params.hpp>

#include <miet/lambda/http/base/client.hpp>
#include <miet/lambda/project/base/storage.hpp>

namespace miet::lambda::engine::lua {
struct ModulesDeps final {
  http::ClientPtr httpClient;
  project::StoragePtr projectStorage;
};

class Executor final : public ExecutorBase {
 public:
  explicit Executor(ExecutionControlParams execCtlParams,
                    ModulesDeps deps) noexcept;

  void Execute(ScriptPtr script, ExecutionContextRef context) override;

 private:
  ExecutionControlParams execCtlParams_;
  ModulesDeps modulesDeps_;
};
}  // namespace miet::lambda::engine::lua
