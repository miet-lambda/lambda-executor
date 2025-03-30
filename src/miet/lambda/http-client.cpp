#include <miet/lambda/http-client.hpp>

constexpr userver::clients::http::HttpMethod ToClientMethod(
    miet::lambda::http::Request::HttpMethod method) {
  switch (method) {
    case miet::lambda::http::Request::HttpMethod::kDelete:
      return userver::clients::http::HttpMethod::kDelete;
    case miet::lambda::http::Request::HttpMethod::kGet:
      return userver::clients::http::HttpMethod::kGet;
    case miet::lambda::http::Request::HttpMethod::kHead:
      return userver::clients::http::HttpMethod::kHead;
    case miet::lambda::http::Request::HttpMethod::kPost:
      return userver::clients::http::HttpMethod::kPost;
    case miet::lambda::http::Request::HttpMethod::kPut:
      return userver::clients::http::HttpMethod::kPut;
    case miet::lambda::http::Request::HttpMethod::kPatch:
      return userver::clients::http::HttpMethod::kPatch;
    case miet::lambda::http::Request::HttpMethod::kOptions:
      return userver::clients::http::HttpMethod::kOptions;
    default:
      throw std::runtime_error("Unknown request method");
  }
  return userver::clients::http::HttpMethod::kGet;
}

namespace miet::lambda::http {
Client::Client(NativeClient& client) noexcept : client_(client) {}

http::Response Client::Send(const http::Request& request) {
  Response unifiedResponse;
  auto response =
      client_.CreateRequest()
          .method(ToClientMethod(request.GetMethod()))
          .url(request.GetUrl())
          .data(request.GetBody().data())
          .headers(*request.GetHeaders())
          .timeout(std::chrono::seconds(5)) /** @todo pass custom timeout */
          .perform();
  unifiedResponse.SetStatus(response->status_code());
  unifiedResponse.SetHeaders(
      std::make_shared<userver::http::headers::HeaderMap>(
          std::move(response)->headers()));
  unifiedResponse.SetBody(std::move(response)->body());
  return unifiedResponse;
}
}  // namespace miet::lambda::http
