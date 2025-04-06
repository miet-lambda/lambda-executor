#pragma once

#include <miet/lambda/http.hpp>

#include <userver/utils/not_null.hpp>

namespace miet::lambda {
struct ExecuteOptions final {
  std::chrono::milliseconds timeout;
};

class ExecutionContext final {
 public:
  ExecutionContext(http::Request request, http::Response response) noexcept;

  const http::Request& GetRequest() const noexcept;
  const http::Response& GetResponse() const noexcept;
  const ExecuteOptions& GetOptions() const noexcept;
  http::Request& GetRequest() noexcept;
  http::Response& GetResponse() noexcept;
  ExecuteOptions& GetOptions() noexcept;

 private:
  http::Request request_;
  http::Response response_;
  ExecuteOptions options_;
};

using ExecutionContextRef = userver::utils::SharedRef<ExecutionContext>;
}  // namespace miet::lambda
