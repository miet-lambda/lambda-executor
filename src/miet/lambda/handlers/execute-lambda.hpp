#pragma once

#include <miet/lambda/engine/base/executor.hpp>

#include <miet/lambda/scripts-storage.hpp>

#include <userver/server/handlers/http_handler_base.hpp>

namespace miet::lambda::handlers {
class ExecuteLambda final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-execute-lambda";

  ExecuteLambda(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context);

  [[nodiscard]] engine::ScriptPtr GetScriptById(std::string_view id) const;
  void ExecuteScript(engine::ScriptPtr script,
                     ExecutionContextRef context) const;

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext& context) const override;

 private:
  engine::ExecutorPtr executor_ = nullptr;
  ScriptsStoragePtr scriptsStorage_ = nullptr;
};
}  // namespace miet::lambda::handlers
