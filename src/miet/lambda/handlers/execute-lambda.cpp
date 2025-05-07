#include <miet/lambda/handlers/execute-lambda.hpp>

#include <miet/lambda/components/script-engine.hpp>
#include <miet/lambda/components/scripts-storage-provider.hpp>
#include <miet/lambda/exceptions.hpp>
#include <miet/lambda/execution-context.hpp>

#include <userver/components/component.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/handlers/exceptions.hpp>

#include <boost/lexical_cast.hpp>

constexpr auto kDefaultExecutionTimeout = std::chrono::seconds(1);
constexpr auto kDefaultMemoryLimit = 10 * 1024 * 1024;  // 10 MiB

template <typename ClientError>
void RethrowWithExternalMessage(std::string_view message) {
  userver::formats::json::ValueBuilder errorBuilder;
  errorBuilder.EmplaceNocheck("message", message);
  throw ClientError(userver::server::handlers::ExternalBody{
      .body = userver::formats::json::ToString(errorBuilder.ExtractValue())});
}

namespace miet::lambda::handlers {
ExecuteLambda::ExecuteLambda(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {
  executor_ = context.FindComponent<components::ScriptEngine>().GetExecutor();
  scriptsStorage_ =
      context.FindComponent<components::ScriptsStorageProvider>().GetStorage();
}

engine::ScriptPtr ExecuteLambda::GetScriptById(std::string_view id) const {
  engine::ScriptIdType scriptId;
  try {
    scriptId = boost::lexical_cast<engine::ScriptIdType>(id);
  } catch (...) {
    RethrowWithExternalMessage<userver::server::handlers::ClientError>(
        "Incorrect script ID");
  }
  try {
    return scriptsStorage_->Get(scriptId);
  } catch (const NotFoundScriptError& ex) {
    RethrowWithExternalMessage<userver::server::handlers::ResourceNotFound>(
        ex.what());
  } catch (const std::exception& ex) {
    LOG_ERROR() << "Can't get script from storage: " << ex.what();
    throw userver::server::handlers::InternalServerError();
  }
  UINVARIANT(false, "(-_-)");
  return nullptr;
}

static ExecutionContextRef BuildExecutionContext(
    const userver::server::http::HttpRequest& request) {
  http::Request scriptRequest;
  try {
    const auto jsonBody =
        userver::formats::json::FromString(request.RequestBody());
    scriptRequest = http::Request::FromJson(jsonBody);
  } catch (const std::exception& ex) {
    RethrowWithExternalMessage<userver::server::handlers::ClientError>(
        ex.what());
  }

  const auto execContext = userver::utils::MakeSharedRef<ExecutionContext>(
      std::move(scriptRequest), http::Response::Default());

  execContext->GetOptions().timeout = kDefaultExecutionTimeout;
  execContext->GetOptions().memoryLimit = kDefaultMemoryLimit;

  return execContext;
}

void ExecuteLambda::ExecuteScript(engine::ScriptPtr script,
                                  ExecutionContextRef context) const {
  try {
    executor_->Execute(script, std::move(context));
  } catch (const ExecutionTimeout& ex) {
    LOG_WARNING() << "Execution expired: " << ex.what();
    /** @todo Select other http code */
    throw userver::server::handlers::InternalServerError();
  } catch (const OutOfMemoryLimit& ex) {
    LOG_WARNING() << "Out of memory while executing the script: " << ex.what();
    /** @todo Select other http code */
    throw userver::server::handlers::InternalServerError();
  } catch (const NotFoundScriptError& ex) {
    LOG_WARNING() << "Can't find script: " << ex.what();
    throw userver::server::handlers::ResourceNotFound();
  } catch (const std::exception& ex) {
    LOG_ERROR() << "Execution exception: " << ex.what();
    /** @todo Select other http code */
    throw userver::server::handlers::InternalServerError();
  } catch (...) {
    LOG_ERROR() << "Unknown execution exception";
    throw userver::server::handlers::InternalServerError();
  }
}

std::string ExecuteLambda::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    [[maybe_unused]] userver::server::request::RequestContext& context) const {
  const auto script = GetScriptById(request.GetPathArg("id"));

  const auto execContext = BuildExecutionContext(request);

  ExecuteScript(script, execContext);

  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  return userver::formats::json::ToString(execContext->GetResponse().ToJson());
}
}  // namespace miet::lambda::handlers
