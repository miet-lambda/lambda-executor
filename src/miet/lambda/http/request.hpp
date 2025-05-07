#pragma once

#include <miet/lambda/http/message.hpp>

#include <userver/formats/json/value.hpp>
#include <userver/server/http/http_method.hpp>

#include <unordered_map>

namespace miet::lambda::http {
class Request final : public Message {
 public:
  using HttpMethod = userver::server::http::HttpMethod;
  using QueryParamsPtr =
      std::shared_ptr<std::unordered_map<std::string, std::string>>;

  static Request FromJson(userver::formats::json::Value json);
  static Request Default() noexcept;

  void SetMethod(HttpMethod method) noexcept;
  void SetUrl(std::string url) noexcept;
  void SetQueryParams(QueryParamsPtr query) noexcept;

  HttpMethod GetMethod() const noexcept;
  const std::string& GetUrl() const noexcept;
  QueryParamsPtr GetQueryParams() const noexcept;

 private:
  HttpMethod method_;
  std::string url_;
  QueryParamsPtr query_;
};
}  // namespace miet::lambda::http
