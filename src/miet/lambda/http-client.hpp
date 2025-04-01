#pragma once

#include <miet/lambda/base/http-client.hpp>

#include <userver/clients/http/client.hpp>

namespace miet::lambda::http {
class Client final : public HttpClientBase {
 public:
  using NativeClient = userver::clients::http::Client;

  explicit Client(NativeClient& client) noexcept;

  http::Response Send(const http::Request& request) override;

 private:
  NativeClient& client_;
};
}  // namespace miet::lambda::http
