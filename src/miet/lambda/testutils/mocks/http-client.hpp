#pragma once

#include <miet/lambda/http/base/client.hpp>

#include <gmock/gmock.h>

namespace miet::lambda::http {
class ClientMock final : public ClientBase {
 public:
  MOCK_METHOD(Response, Send, (const Request&));
};
}  // namespace miet::lambda::http
