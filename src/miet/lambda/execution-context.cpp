#include <miet/lambda/execution-context.hpp>

namespace miet::lambda {
ExecutionContext::ExecutionContext(http::Request request,
                                   http::Response response) noexcept
    : request_(std::move(request)), response_(std::move(response)) {}

const http::Request& ExecutionContext::GetRequest() const noexcept {
  return request_;
}

const http::Response& ExecutionContext::GetResponse() const noexcept {
  return response_;
}

http::Request& ExecutionContext::GetRequest() noexcept { return request_; }

http::Response& ExecutionContext::GetResponse() noexcept { return response_; }
}  // namespace miet::lambda
