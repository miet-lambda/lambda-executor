#include <miet/lambda/http.hpp>

#include <userver/utest/utest.hpp>

using namespace miet::lambda;

UTEST(Response, Default) {
  const auto request = http::Response::Default();
  ASSERT_EQ(request.GetStatus(), http::Response::HttpStatus::kOk);
  ASSERT_EQ(request.GetHeaders()->size(), 0);
  ASSERT_EQ(request.GetBody(), "");
}
