#pragma once

#include <miet/lambda/http/request.hpp>
#include <miet/lambda/http/response.hpp>

namespace miet::lambda::http {
class ClientBase {
 public:
  virtual ~ClientBase() = default;

  virtual Response Send(const Request& request) = 0;
};

using ClientPtr = std::shared_ptr<ClientBase>;
}  // namespace miet::lambda::http
