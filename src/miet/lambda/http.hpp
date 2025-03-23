#pragma once

#include <userver/formats/json/value.hpp>
#include <userver/http/header_map.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_status.hpp>

#include <unordered_map>

namespace miet::lambda::http {
class Message {
 public:
  using HeadersMapPtr = std::shared_ptr<userver::http::headers::HeaderMap>;

  HeadersMapPtr GetHeaders() const noexcept;
  void SetHeaders(HeadersMapPtr headers) noexcept;

  std::string_view GetBody() const noexcept;
  void SetBody(std::string body) noexcept;

 private:
  HeadersMapPtr headers_;
  std::string body_;
};

class Request final : public Message {
 public:
  using HttpMethod = userver::server::http::HttpMethod;
  using QueryParamsPtr =
      std::shared_ptr<std::unordered_map<std::string, std::string>>;

  static Request FromJson(userver::formats::json::Value json);
  static Request Default() noexcept;

  HttpMethod GetMethod() const noexcept;
  void SetMethod(HttpMethod method) noexcept;

  const std::string& GetUrl() const noexcept;
  void SetUrl(std::string url) noexcept;

  QueryParamsPtr GetQueryParams() const noexcept;
  void SetQueryParams(QueryParamsPtr query) noexcept;

 private:
  HttpMethod method_;
  std::string url_;
  QueryParamsPtr query_;
};

class Response final : public Message {
 public:
  using HttpStatus = userver::server::http::HttpStatus;

  static Response Default() noexcept;

  HttpStatus GetStatus() const noexcept;
  void SetStatus(HttpStatus status) noexcept;

 private:
  HttpStatus status_;
};

}  // namespace miet::lambda::http
