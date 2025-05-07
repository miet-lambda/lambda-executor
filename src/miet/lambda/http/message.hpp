#pragma once

#include <userver/http/header_map.hpp>

namespace miet::lambda::http {
class Message {
 public:
  using HeadersMapPtr = std::shared_ptr<userver::http::headers::HeaderMap>;

  void SetHeaders(HeadersMapPtr headers) noexcept;
  void SetBody(std::string body) noexcept;

  HeadersMapPtr GetHeaders() const noexcept;
  std::string_view GetBody() const noexcept;

 private:
  HeadersMapPtr headers_;
  std::string body_;
};
}  // namespace miet::lambda::http
