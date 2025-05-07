#pragma once

#include <miet/lambda/http/message.hpp>

#include <userver/formats/json/value.hpp>
#include <userver/server/http/http_status.hpp>

namespace miet::lambda::http {
class Response final : public Message {
 public:
  using HttpStatus = userver::server::http::HttpStatus;

  static Response Default() noexcept;

  void SetStatus(HttpStatus status) noexcept;

  HttpStatus GetStatus() const noexcept;

  userver::formats::json::Value ToJson() const noexcept;

 private:
  HttpStatus status_;
};
}  // namespace miet::lambda::http
