#include <miet/lambda/lua/http-context.hpp>

#include <miet/lambda/lua/utils.hpp>

#include <userver/logging/log.hpp>

#include <LuaCpp/LuaCpp.hpp>

constexpr auto kRequestVariableName = "miet_http_context_incoming_request";
constexpr auto kResponseVariableName = "miet_http_context_outgoing_response";

constexpr auto kRequestMethodName = "request";
constexpr auto kResponseMethodName = "response";

constexpr auto kStatusKey = "status";
constexpr auto kMethodKey = "method";
constexpr auto kUrlKey = "url";
constexpr auto kQueryParamsKey = "query";
constexpr auto kHeadersKey = "headers";
constexpr auto kBodyKey = "body";

namespace miet::lambda::lua {
namespace {
struct AccessMethod final : public LuaCpp::LuaMetaObject {
  explicit AccessMethod(std::string globalVariableName)
      : globalVariableName_(std::move(globalVariableName)) {}

  int Execute(LuaCpp::Engine::LuaState& L) override {
    const auto n = lua_gettop(L);
    if (n != 2) {
      return luaL_error(
          L,
          "Invalid arguments count (expected 2 (class, object), provided %d)",
          n);
    }
    lua_getglobal(L, globalVariableName_.c_str());
    return 1;
  }

 private:
  std::string globalVariableName_;
};
}  // namespace

static std::shared_ptr<LuaCpp::Engine::LuaTTable> PopulateLuaMassageFrom(
    const http::Message& message) {
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
    http::Request&& request) {
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
    http::Response&& response) {
  const auto luaResponse = PopulateLuaMassageFrom(response);
  luaResponse->setValue(LuaCpp::Engine::Table::Key(kStatusKey),
                        std::make_shared<LuaCpp::Engine::LuaTNumber>(
                            static_cast<std::int32_t>(response.GetStatus())));
  return luaResponse;
}

static http::Response PopulateNativeResponseFrom(
    std::shared_ptr<LuaCpp::Engine::LuaTTable> luaResponse) {
  http::Response response;

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

HttpContext::HttpContext(LuaContextWeakPtr luaContext,
                         ExecutionContextRef executionContext)
    : luaContext_(std::move(luaContext)),
      executionContext_(std::move(executionContext)) {
  auto luaRequest =
      PopulateLuaRequestFrom(std::move(executionContext_->GetRequest()));
  auto luaResponse =
      PopulateLuaResponseFrom(std::move(executionContext_->GetResponse()));

  const auto lock = luaContext_.lock();
  lock->AddGlobalVariable(kRequestVariableName, std::move(luaRequest));
  lock->AddGlobalVariable(kResponseVariableName, std::move(luaResponse));
  getRequestMethod_ = std::make_shared<AccessMethod>(kRequestVariableName);
  getResponseMethod_ = std::make_shared<AccessMethod>(kResponseVariableName);
}

void HttpContext::PopulateResponse() {
  const auto luaResponse =
      luaContext_.lock()->getGlobalVariable(kResponseVariableName);
  executionContext_->GetResponse() = PopulateNativeResponseFrom(
      utils::GetValueStrict<LuaCpp::Engine::LuaTTable>(luaResponse));
}

std::shared_ptr<LuaCpp::Engine::LuaType> HttpContext::getValue(
    std::string& key) {
  if (key == kRequestMethodName) {
    return getRequestMethod_;
  } else if (key == kResponseMethodName) {
    return getResponseMethod_;
  }
  return std::make_shared<LuaCpp::Engine::LuaTNil>();
}
}  // namespace miet::lambda::lua
