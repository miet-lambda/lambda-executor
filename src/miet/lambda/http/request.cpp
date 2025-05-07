#include <miet/lambda/http/request.hpp>

namespace miet::lambda::http {
void PopulateMessageDefault(Message& message);
void PopulateMessageFromJson(Message& message,
                             userver::formats::json::Value json);

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

}  // namespace miet::lambda::http
