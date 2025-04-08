#include <miet/lambda/handlers/execute-lambda.hpp>

#include <miet/lambda/components/executor.hpp>
#include <miet/lambda/exceptions.hpp>
#include <miet/lambda/execution-context.hpp>

#include <userver/components/component.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/handlers/exceptions.hpp>

constexpr auto kDefaultExecutionTimeout = std::chrono::seconds(1);

namespace miet::lambda::handlers {
ExecuteLambda::ExecuteLambda(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {
  executor_ = context.FindComponent<components::Executor>().GetExecutor();
}

std::string ExecuteLambda::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    [[maybe_unused]] userver::server::request::RequestContext& context) const {
  const auto scriptId = request.GetPathArg("id");

  http::Request scriptRequest;
  try {
    const auto jsonBody =
        userver::formats::json::FromString(request.RequestBody());
    scriptRequest = http::Request::FromJson(jsonBody);
  } catch (const std::exception& ex) {
    userver::formats::json::ValueBuilder errorBuilder;
    errorBuilder.EmplaceNocheck("message", ex.what());
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            .body =
                userver::formats::json::ToString(errorBuilder.ExtractValue())});
  }

  const auto executionContext = userver::utils::MakeSharedRef<ExecutionContext>(
      std::move(scriptRequest), http::Response::Default());

  executionContext->GetOptions().timeout = kDefaultExecutionTimeout;

  try {
    executor_->Execute(scriptId, executionContext);
  } catch (const ExecutionTimout& ex) {
    LOG_WARNING() << "Execution expired: " << ex.what();
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kRequestTimeout);
    return {};
  } catch (const NotFoundScriptError& ex) {
    LOG_WARNING() << "Can't find script: " << ex.what();
    throw userver::server::handlers::ResourceNotFound();
  } catch (const std::exception& ex) {
    LOG_ERROR() << "Execution exception: " << ex.what();
    throw userver::server::handlers::InternalServerError();
  } catch (...) {
    LOG_ERROR() << "Unkonw execution exception";
    throw userver::server::handlers::InternalServerError();
  }
  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);
  return userver::formats::json::ToString(
      executionContext->GetResponse().ToJson());
}
}  // namespace miet::lambda::handlers
