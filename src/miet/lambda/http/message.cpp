#include <miet/lambda/http/message.hpp>

#include <userver/formats/json/value.hpp>

namespace miet::lambda::http {
void PopulateMessageDefault(Message& message) {
  message.SetBody("");
  message.SetHeaders(std::make_shared<userver::http::headers::HeaderMap>());
}

void PopulateMessageFromJson(Message& message,
                             userver::formats::json::Value json) {
  message.SetBody(json["body"].As<std::string>(""));

  auto headers = std::make_shared<userver::http::headers::HeaderMap>();
  if (json.HasMember("headers")) {
    for (auto header = json["headers"].begin(); header != json["headers"].end();
         ++header) {
      headers->emplace(header.GetName(), header->As<std::string>());
    }
  }
  message.SetHeaders(std::move(headers));
}

Message::HeadersMapPtr Message::GetHeaders() const noexcept { return headers_; }

void Message::SetHeaders(HeadersMapPtr headers) noexcept {
  headers_ = std::move(headers);
}

std::string_view Message::GetBody() const noexcept { return body_; }

void Message::SetBody(std::string body) noexcept { body_ = std::move(body); }
}  // namespace miet::lambda::http
