#pragma once

#include <miet/lambda/http/base/client.hpp>

#include <userver/clients/http/client.hpp>

namespace miet::lambda::http {
class Client final : public ClientBase {
 public:
  using NativeClient = userver::clients::http::Client;

  explicit Client(NativeClient& client) noexcept;

  Response Send(const Request& request) override;

 private:
  NativeClient& client_;
};
}  // namespace miet::lambda::http
