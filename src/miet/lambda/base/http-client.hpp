#pragma once

#include <miet/lambda/http.hpp>

namespace miet::lambda {
class HttpClientBase {
 public:
  ~HttpClientBase() = default;

  virtual http::Response Send(const http::Request& request) = 0;
};

using HttpClientPtr = std::shared_ptr<HttpClientBase>;
}  // namespace miet::lambda
