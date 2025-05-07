#include <miet/lambda/http/request.hpp>

#include <miet/lambda/testutils/utils.hpp>

#include <userver/formats/json/schema.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/utest/utest.hpp>

using namespace miet::lambda;

UTEST(Request, Default) {
  const auto request = http::Request::Default();
  ASSERT_EQ(request.GetMethod(), http::Request::HttpMethod::kGet);
  ASSERT_EQ(request.GetUrl(), "");
  ASSERT_EQ(request.GetQueryParams()->size(), 0);
  ASSERT_EQ(request.GetHeaders()->size(), 0);
  ASSERT_EQ(request.GetBody(), "");
}

UTEST(Request, FromJson) {
  constexpr auto kStringRequest = R"(
    {
      "method": "PATCH",
      "url": "/miet/lambda-executor/lua",
      "query": {
        "key": "value",
        "format": "json"
      },
      "headers": {
        "Content-Type": "application/json",
        "X-My-Header": "value"
      },
      "body": "some-data"
    }
  )";

  const auto jsonRequest = userver::formats::json::FromString(kStringRequest);
  const auto request = http::Request::FromJson(jsonRequest);
  ASSERT_EQ(request.GetMethod(), http::Request::HttpMethod::kPatch);
  ASSERT_EQ(request.GetUrl(), "/miet/lambda-executor/lua");
  ASSERT_EQ(request.GetQueryParams()->size(), 2);
  ASSERT_EQ(request.GetQueryParams()->at("key"), "value");
  ASSERT_EQ(request.GetQueryParams()->at("format"), "json");
  ASSERT_EQ(request.GetHeaders()->size(), 2);
  ASSERT_EQ(request.GetHeaders()->at(userver::http::headers::kContentType),
            "application/json");
  ASSERT_EQ(request.GetHeaders()->at(kMyHeader), "value");
  ASSERT_EQ(request.GetBody(), "some-data");
}
