#include <miet/lambda/lua/executor.hpp>

#include <miet/lambda/testutils/mocks/scripts-fetcher.hpp>
#include <miet/lambda/testutils/utils.hpp>

#include <userver/http/common_headers.hpp>
#include <userver/utest/utest.hpp>

using namespace miet::lambda;

class TestLuaExecutor : public testing::Test {
 protected:
  void SetUp() override {
    fetcher_ = std::make_shared<ScriptsFetcherMock>();
    executor_ = std::make_shared<lua::Executor>(fetcher_);
  }

 protected:
  std::shared_ptr<ScriptsFetcherMock> fetcher_ = nullptr;
  std::shared_ptr<lua::Executor> executor_ = nullptr;
};

UTEST_F(TestLuaExecutor, WithoutContext) {
  EXPECT_CALL(*fetcher_, Fetch("without-context"))
      .WillOnce(::testing::Return(R"(
    function my_sum(a, b)
      return a + b
    end

    local total_sum = 0
    for i = 1, 10 do
      total_sum = my_sum(total_sum, i)
    end

    if total_sum ~= 55 then
      error('Invalid sum expected 55')
    end
  )"));

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("without-context", context));
}

UTEST_F(TestLuaExecutor, JsonEncodeDecode) {
  EXPECT_CALL(*fetcher_, Fetch("json-encode-decode"))
      .WillOnce(::testing::Return(R"(
    local json = require("dkjson")

    local data = {
        name = "Lua",
        version = "5.4",
        features = {"lightweight", "embeddable", "fast"}
    }

    local json_text = json.encode(data, { indent = true })

    local decoded_data, pos, err = json.decode(json_text, 1, nil)
    if err then
        error('Decode error: ' .. err)
    end
    
    for key, value in pairs(decoded_data) do
        --- some operations
    end
  )"));

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("json-encode-decode", context));
}

UTEST_F(TestLuaExecutor, IncomingRequest) {
  EXPECT_CALL(*fetcher_, Fetch("incoming-request"))
      .WillOnce(::testing::Return(R"(
    local context = require('miet.http.context').get()

    local request = context:request()

    if request['method'] ~= 'PATCH' then
      error('Incorrect method')
    end

    if request['url'] ~= '/miet/lambda-executor/lua' then
      error('Incorrect url')
    end

    local query_params = request['query']
    if query_params['key'] ~= 'value' or query_params['format'] ~= 'json' then
      error('Incorrect query parameters')
    end

    local headers = request['headers']
    if headers['Content-Type'] ~= 'application/json' or headers['X-My-Header'] ~= 'value' then
      error('Incorrect headers')
    end

    if request['body'] ~= 'some-data' then
      error('Incorrect body')
    end
  )"));

  constexpr auto kRequest = R"(
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

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::FromJson(userver::formats::json::FromString(kRequest)),
      http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("incoming-request", context));
}

UTEST_F(TestLuaExecutor, OutgoingResponse) {
  EXPECT_CALL(*fetcher_, Fetch("outgoing-response"))
      .WillOnce(::testing::Return(R"(
    local context = require('miet.http.context').get()

    local response = context:response()

    response['status'] = 201
    response['headers'] = {
      ['Content-Type'] = 'plain/text',
      ['X-My-Header'] = 'some-value'
    }
    response['body'] = {
      name = 'Alex',
      age = 18
    }
  )"));

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("outgoing-response", context));

  const auto& response = context->GetResponse();
  ASSERT_EQ(response.GetStatus(), http::Response::HttpStatus::kCreated);
  ASSERT_EQ(response.GetHeaders()->size(), 2);
  ASSERT_EQ(response.GetHeaders()->at(userver::http::headers::kContentType),
            "plain/text");
  ASSERT_EQ(response.GetHeaders()->at(kMyHeader), "some-value");

  const auto jsonBody = userver::formats::json::FromString(response.GetBody());
  ASSERT_EQ(jsonBody["name"].As<std::string>(), "Alex");
  ASSERT_EQ(jsonBody["age"].As<std::uint64_t>(), 18);
}
