#include <miet/lambda/http/response.hpp>

#include <userver/formats/json.hpp>
#include <userver/utest/utest.hpp>

using namespace miet::lambda;

UTEST(Response, Default) {
  const auto request = http::Response::Default();
  ASSERT_EQ(request.GetStatus(), http::Response::HttpStatus::kOk);
  ASSERT_EQ(request.GetHeaders()->size(), 0);
  ASSERT_EQ(request.GetBody(), "");
}

UTEST(Response, DefaultToJson) {
  const auto response = http::Response::Default();
  const auto json = response.ToJson();

  ASSERT_EQ(json["status"].As<std::int32_t>(), 200);
  ASSERT_FALSE(json.HasMember("headers"));
  ASSERT_EQ(json["body"].As<std::string>(), "");
}

UTEST(Response, CustomToJson) {
  auto response = http::Response::Default();
  response.SetStatus(http::Response::HttpStatus::kCreated);
  response.SetBody("test body");

  userver::http::headers::HeaderMap headersMap = {
      {"Content-Type", "application/json"}, {"X-Custom-Header", "value"}};
  response.SetHeaders(std::make_shared<userver::http::headers::HeaderMap>(
      std::move(headersMap)));

  const auto json = response.ToJson();

  ASSERT_EQ(json["status"].As<std::int32_t>(), 201);
  ASSERT_EQ(json["body"].As<std::string>(), "test body");

  const auto headers = json["headers"];
  ASSERT_EQ(headers["Content-Type"].As<std::string>(), "application/json");
  ASSERT_EQ(headers["X-Custom-Header"].As<std::string>(), "value");
}
