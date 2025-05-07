#include <miet/lambda/engine/lua/modules/http/context.hpp>

#include <miet/lambda/engine/lua/utils.hpp>

constexpr auto kRequestVariableName = "miet_http_context_incoming_request";
constexpr auto kResponseVariableName = "miet_http_context_outgoing_response";

constexpr auto kStatusKey = "status";
constexpr auto kMethodKey = "method";
constexpr auto kUrlKey = "url";
constexpr auto kQueryParamsKey = "query";
constexpr auto kHeadersKey = "headers";
constexpr auto kBodyKey = "body";

namespace miet::lambda::engine::lua::modules::http {
static std::shared_ptr<LuaCpp::Engine::LuaTTable> PopulateLuaMassageFrom(
    const miet::lambda::http::Message& message) {
  const auto luaMessage = std::make_shared<LuaCpp::Engine::LuaTTable>();

  /** @note Populate request headers */
  auto luaHeaders = std::make_shared<LuaCpp::Engine::LuaTTable>();
  for (auto&& [header, value] : *message.GetHeaders()) {
    luaHeaders->setValue(
        LuaCpp::Engine::Table::Key(header),
        std::make_shared<LuaCpp::Engine::LuaTString>(std::move(value)));
  }
  luaMessage->setValue(LuaCpp::Engine::Table::Key(kHeadersKey),
                       std::move(luaHeaders));

  /** @note Populate request body */
  luaMessage->setValue(
      LuaCpp::Engine::Table::Key(kBodyKey),
      std::make_shared<LuaCpp::Engine::LuaTString>(message.GetBody().data()));
  return luaMessage;
}

static std::shared_ptr<LuaCpp::Engine::LuaTTable> PopulateLuaRequestFrom(
    miet::lambda::http::Request&& request) {
  const auto luaRequest = PopulateLuaMassageFrom(request);

  /** @note Populate request method */
  luaRequest->setValue(
      LuaCpp::Engine::Table::Key(kMethodKey),
      std::make_shared<LuaCpp::Engine::LuaTString>(
          userver::server::http::ToString(request.GetMethod())));

  /** @note Populate request url */
  luaRequest->setValue(
      LuaCpp::Engine::Table::Key(kUrlKey),
      std::make_shared<LuaCpp::Engine::LuaTString>(request.GetUrl()));

  /** @note Populate request query parameters */
  auto luaQueryParams = std::make_shared<LuaCpp::Engine::LuaTTable>();
  for (auto&& [key, value] : *request.GetQueryParams()) {
    luaQueryParams->setValue(
        LuaCpp::Engine::Table::Key(key),
        std::make_shared<LuaCpp::Engine::LuaTString>(std::move(value)));
  }
  luaRequest->setValue(LuaCpp::Engine::Table::Key(kQueryParamsKey),
                       std::move(luaQueryParams));
  return luaRequest;
}

static std::shared_ptr<LuaCpp::Engine::LuaTTable> PopulateLuaResponseFrom(
    miet::lambda::http::Response&& response) {
  const auto luaResponse = PopulateLuaMassageFrom(response);
  luaResponse->setValue(LuaCpp::Engine::Table::Key(kStatusKey),
                        std::make_shared<LuaCpp::Engine::LuaTNumber>(
                            static_cast<std::int32_t>(response.GetStatus())));
  return luaResponse;
}

static miet::lambda::http::Response PopulateNativeResponseFrom(
    std::shared_ptr<LuaCpp::Engine::LuaTTable> luaResponse) {
  miet::lambda::http::Response response;

  /** @note Populate response status */
  const auto statusCode =
      utils::GetValueStrict<LuaCpp::Engine::LuaTNumber>(
          &luaResponse->getValue(LuaCpp::Engine::Table::Key(kStatusKey)))
          ->getValue();
  response.SetStatus(
      static_cast<userver::server::http::HttpStatus>(statusCode));

  /** @note Populate response headers */
  const auto* luaHeaders = utils::GetValueStrict<LuaCpp::Engine::LuaTTable>(
      &luaResponse->getValue(LuaCpp::Engine::Table::Key(kHeadersKey)));
  auto headers = std::make_shared<userver::http::headers::HeaderMap>();
  for (const auto& [header, value] : luaHeaders->getValues()) {
    headers->emplace(header.ToString(), value->ToString());
  }
  response.SetHeaders(std::move(headers));

  /** @note Populate response body */
  response.SetBody(
      luaResponse->getValue(LuaCpp::Engine::Table::Key(kBodyKey)).ToString());
  return response;
}

Context::Context(LuaEnvRef luaEnv, ExecutionContextRef executionContext)
    : luaEnv_(luaEnv), executionContext_(std::move(executionContext)) {
  auto luaRequest =
      PopulateLuaRequestFrom(std::move(executionContext_->GetRequest()));
  auto luaResponse =
      PopulateLuaResponseFrom(std::move(executionContext_->GetResponse()));

  luaEnv_.emplace(kRequestVariableName, std::move(luaRequest));
  luaEnv_.emplace(kResponseVariableName, std::move(luaResponse));
}

Context::~Context() {
  const auto luaResponse = luaEnv_.at(kResponseVariableName);
  executionContext_->GetResponse() = PopulateNativeResponseFrom(
      utils::GetValueStrict<LuaCpp::Engine::LuaTTable>(luaResponse));
}
}  // namespace miet::lambda::engine::lua::modules::http
