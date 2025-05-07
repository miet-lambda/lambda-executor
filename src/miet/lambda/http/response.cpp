#include <miet/lambda/http/response.hpp>

#include <userver/formats/json/value_builder.hpp>

namespace miet::lambda::http {
void PopulateMessageDefault(Message& message);

Response Response::Default() noexcept {
  Response response;
  PopulateMessageDefault(response);
  response.SetStatus(Response::HttpStatus::kOk);
  return response;
}

Response::HttpStatus Response::GetStatus() const noexcept { return status_; }

void Response::SetStatus(Response::HttpStatus status) noexcept {
  status_ = status;
}

userver::formats::json::Value Response::ToJson() const noexcept {
  userver::formats::json::ValueBuilder builder;
  builder.EmplaceNocheck("status", static_cast<std::uint16_t>(status_));
  {
    userver::formats::json::ValueBuilder headersBuilder;
    for (const auto& [header, value] : *GetHeaders()) {
      headersBuilder.EmplaceNocheck(header, value);
    }
    if (!headersBuilder.IsNull()) {
      builder.EmplaceNocheck("headers", std::move(headersBuilder));
    }
  }
  builder.EmplaceNocheck("body", GetBody());
  return builder.ExtractValue();
}
}  // namespace miet::lambda::http
