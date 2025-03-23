#pragma once

#include <miet/lambda/base/executor.hpp>

#include <userver/server/handlers/http_handler_base.hpp>

namespace miet::lambda::handlers {
class ExecuteLambda final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-execute-lambda";

  ExecuteLambda(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext& context) const override;

 private:
  ExecutorPtr executor_ = nullptr;
};
}  // namespace miet::lambda::handlers
