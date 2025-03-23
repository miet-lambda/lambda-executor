#include <miet/lambda/http.hpp>

#include <userver/formats/json/value_builder.hpp>

namespace miet::lambda::http {
static void PopulateMessageFromJson(Message& message,
                                    userver::formats::json::Value json) {
  message.SetBody(json["body"].As<std::string>(""));

  auto headers = std::make_shared<userver::http::headers::HeaderMap>();
  if (json.HasMember("headers")) {
    for (auto header = json["headers"].begin(); header != json["headers"].end();
         ++header) {
      headers->emplace(header.GetName(), header->As<std::string>());
    }
  }
  message.SetHeaders(std::move(headers));
}

static void PopulateMessageDefault(Message& message) {
  message.SetBody("");
  message.SetHeaders(std::make_shared<userver::http::headers::HeaderMap>());
}

Message::HeadersMapPtr Message::GetHeaders() const noexcept { return headers_; }

void Message::SetHeaders(HeadersMapPtr headers) noexcept {
  headers_ = std::move(headers);
}

std::string_view Message::GetBody() const noexcept { return body_; }

void Message::SetBody(std::string body) noexcept { body_ = std::move(body); }

Request Request::Default() noexcept {
  Request request;
  PopulateMessageDefault(request);
  request.SetMethod(Request::HttpMethod::kGet);
  request.SetQueryParams(
      std::make_shared<std::unordered_map<std::string, std::string>>());
  return request;
}

Request::HttpMethod Request::GetMethod() const noexcept { return method_; }

void Request::SetMethod(Request::HttpMethod method) noexcept {
  method_ = method;
}

const std::string& Request::GetUrl() const noexcept { return url_; }

void Request::SetUrl(std::string url) noexcept { url_ = std::move(url); }

Request::QueryParamsPtr Request::GetQueryParams() const noexcept {
  return query_;
}

void Request::SetQueryParams(Request::QueryParamsPtr query) noexcept {
  query_ = std::move(query);
}

Request Request::FromJson(userver::formats::json::Value json) {
  Request request;
  PopulateMessageFromJson(request, json);
  request.SetMethod(userver::server::http::HttpMethodFromString(
      json["method"].As<std::string>()));
  request.SetUrl(json["url"].As<std::string>());
  auto queryParams =
      std::make_shared<std::unordered_map<std::string, std::string>>();
  if (json.HasMember("query")) {
    for (auto param = json["query"].begin(); param != json["query"].end();
         ++param) {
      queryParams->emplace(param.GetName(), param->As<std::string>());
    }
  }
  request.SetQueryParams(std::move(queryParams));
  return request;
}

Response Response::Default() noexcept {
  Response response;
  PopulateMessageDefault(response);
  response.SetStatus(Response::HttpStatus::kOk);
  return response;
}

Response::HttpStatus Response::GetStatus() const noexcept { return status_; }

void Response::SetStatus(Response::HttpStatus status) noexcept {
  status_ = status;
}

userver::formats::json::Value Response::ToJson() const noexcept {
  userver::formats::json::ValueBuilder builder;
  builder.EmplaceNocheck("status", static_cast<std::uint16_t>(status_));
  {
    userver::formats::json::ValueBuilder headersBuilder;
    for (const auto& [header, value] : *GetHeaders()) {
      headersBuilder.EmplaceNocheck(header, value);
    }
    if (!headersBuilder.IsNull()) {
      builder.EmplaceNocheck("headers", std::move(headersBuilder));
    }
  }
  builder.EmplaceNocheck("body", GetBody());
  return builder.ExtractValue();
}
}  // namespace miet::lambda::http
