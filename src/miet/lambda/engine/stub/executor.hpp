#pragma once

#include <miet/lambda/engine/base/executor.hpp>

#include <userver/logging/log.hpp>

namespace miet::lambda::engine::stub {
class Executor final : public ExecutorBase {
 public:
  void Execute(ScriptPtr script, ExecutionContextRef context) override {
    UINVARIANT(script, "Script is NULL");
    LOG_INFO() << "Script with id = '" << script->GetScriptId() << "'executed";
    context->GetResponse().SetStatus(http::Response::HttpStatus::kOk);
    context->GetResponse().SetBody("stub");
  }
};
}  // namespace miet::lambda::engine::stub
