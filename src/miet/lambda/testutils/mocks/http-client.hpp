#pragma once

#include <miet/lambda/base/http-client.hpp>

#include <gmock/gmock.h>

namespace miet::lambda {
class HttpClientMock final : public HttpClientBase {
 public:
  MOCK_METHOD(http::Response, Send, (const http::Request&));
};
}  // namespace miet::lambda
