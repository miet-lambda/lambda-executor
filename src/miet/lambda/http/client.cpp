#include <miet/lambda/http/client.hpp>

#include <userver/logging/log.hpp>

#include <sstream>

namespace miet::lambda::http {
static std::string BuildUrlWithQueryParams(const Request& request) {
  std::ostringstream url;
  url << request.GetUrl();
  if (!request.GetQueryParams()->empty()) {
    url << '?';
    bool isFirstParam = true;
    for (const auto& [key, value] : *request.GetQueryParams()) {
      if (isFirstParam) {
        isFirstParam = false;
        url << '&';
      }
      url << key << '=' << value;
    }
  }
  return url.str();
}

Client::Client(NativeClient& client) noexcept : client_(client) {}

Response Client::Send(const Request& request) {
  LOG_DEBUG() << "Query parameters size: " << request.GetQueryParams()->size();
  LOG_DEBUG() << "Url with query parameteres: "
              << BuildUrlWithQueryParams(request);
  LOG_DEBUG() << "Body: " << request.GetBody();

  Response unifiedResponse;
  auto response =
      client_.CreateRequest()
          .data(request.GetBody().data())
          .set_custom_http_request_method(
              userver::server::http::ToString(request.GetMethod()))
          .url(BuildUrlWithQueryParams(request))
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
