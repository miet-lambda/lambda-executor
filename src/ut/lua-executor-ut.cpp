#include <miet/lambda/lua/executor.hpp>

#include <miet/lambda/testutils/mocks/http-client.hpp>
#include <miet/lambda/testutils/mocks/scripts-fetcher.hpp>
#include <miet/lambda/testutils/utils.hpp>

#include <userver/formats/json/schema.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/http/content_type.hpp>
#include <userver/utest/utest.hpp>

using namespace miet::lambda;

class TestLuaExecutor : public testing::Test {
 protected:
  void SetUp() override {
    fetcher_ = std::make_shared<ScriptsFetcherMock>();
    httpClient_ = std::make_shared<HttpClientMock>();
    executor_ = std::make_shared<lua::Executor>(
        fetcher_, lua::Dependencies{.httpClient = httpClient_});
  }

 protected:
  std::shared_ptr<ScriptsFetcherMock> fetcher_ = nullptr;
  std::shared_ptr<HttpClientMock> httpClient_ = nullptr;
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

UTEST_F(TestLuaExecutor, HttpClientSendError) {
  constexpr auto kMessageHeader =
      userver::http::headers::PredefinedHeader("message");
  constexpr auto kSendErrorMessage = "Can't send request";

  EXPECT_CALL(*fetcher_, Fetch("http-client-send-error"))
      .WillOnce(::testing::Return(R"(
    local client = require('miet.http.client').get()
    local context = require('miet.http.context').get()

    local outgoing_response = context:response()

    local response, err = client:send('GET', 'http://localhost:80/v1/error')
    if response ~= nil or err == nil then
      print(response)
      error('Expected send error')
    end

    outgoing_response['headers'] = {
      message = err
    }
  )"));

  EXPECT_CALL(*httpClient_, Send(::testing::_))
      .WillOnce(
          [&]([[maybe_unused]] const http::Request& request) -> http::Response {
            throw std::runtime_error(kSendErrorMessage);
            return {};
          });

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("http-client-send-error", context));

  ASSERT_TRUE(context->GetResponse().GetHeaders()->contains(kMessageHeader));
  ASSERT_EQ(context->GetResponse().GetHeaders()->at(kMessageHeader),
            kSendErrorMessage);
}

UTEST_F(TestLuaExecutor, HttpClientRequest) {
  EXPECT_CALL(*fetcher_, Fetch("http-client-request"))
      .WillOnce(::testing::Return(R"(
    local client = require('miet.http.client').get()

    local response, err = client:send('HEAD', 'http://localhost:80/v1/hello')
    if err ~= nil then
      error('Can\'t send request: ' .. err)
    end

    response, err = client:send('POST', 'http://localhost:80/v1/message', {
        query = {
          key = 'value'
        },
        headers = {
          ['Content-Type'] = 'application/json'
        },
        body = 'Hello, world!'
    })
    if err ~= nil then
      error('Can\'t send request with additional params: ' .. err)
    end
  )"));

  std::shared_ptr<http::Request> requestWithoutParams = nullptr;
  std::shared_ptr<http::Request> requestWithParams = nullptr;

  EXPECT_CALL(*httpClient_, Send(::testing::_))
      .Times(2)
      .WillRepeatedly([&](const http::Request& request) -> http::Response {
        if (!requestWithoutParams) {
          requestWithoutParams = std::make_shared<http::Request>(request);
        } else {
          requestWithParams = std::make_shared<http::Request>(request);
        }
        return {};
      });

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("http-client-request", context));

  ASSERT_EQ(requestWithoutParams->GetMethod(),
            http::Request::HttpMethod::kHead);
  ASSERT_EQ(requestWithoutParams->GetUrl(), "http://localhost:80/v1/hello");

  ASSERT_EQ(requestWithParams->GetMethod(), http::Request::HttpMethod::kPost);
  ASSERT_EQ(requestWithParams->GetUrl(), "http://localhost:80/v1/message");
  ASSERT_EQ(requestWithParams->GetQueryParams()->at("key"), "value");
  ASSERT_EQ(
      requestWithParams->GetHeaders()->at(userver::http::headers::kContentType),
      userver::http::content_type::kApplicationJson);
  ASSERT_EQ(requestWithParams->GetBody(), "Hello, world!");
}

UTEST_F(TestLuaExecutor, HttpClientRequestTableBody) {
  EXPECT_CALL(*fetcher_, Fetch("http-client-request-table-body"))
      .WillOnce(::testing::Return(R"(
    local client = require('miet.http.client').get()

    local response, err = client:get('http://localhost:80/v1/hello', {
      body = {
        name = 'Test',
        age = 18,
        is_male = true,
        extra = {
          float_number = 3.141516,
          string_value = 'string',
          some_array = { 2, 'imposter', 6 }
        }
      }
    })
    if err ~= nil then
      error('Can\'t send request: ' .. err)
    end
  )"));

  userver::formats::json::Value body;

  EXPECT_CALL(*httpClient_, Send(::testing::_))
      .WillOnce([&](const http::Request& request) -> http::Response {
        body = userver::formats::json::FromString(request.GetBody());
        return {};
      });

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(
      executor_->Execute("http-client-request-table-body", context));

  ASSERT_EQ(body["name"].As<std::string>(), "Test");
  ASSERT_EQ(body["age"].As<std::uint64_t>(), 18ull);
  ASSERT_EQ(body["is_male"].As<bool>(), true);
  ASSERT_EQ(body["extra"]["float_number"].As<double>(), 3.141516);
  ASSERT_EQ(body["extra"]["string_value"].As<std::string>(), "string");
  ASSERT_EQ(body["extra"]["some_array"].GetSize(), 3);
  ASSERT_EQ(body["extra"]["some_array"][0].As<std::uint64_t>(), 2ull);
  ASSERT_EQ(body["extra"]["some_array"][1].As<std::string>(), "imposter");
  ASSERT_EQ(body["extra"]["some_array"][2].As<std::uint64_t>(), 6ull);
}

UTEST_F(TestLuaExecutor, HttpClientResponse) {
  EXPECT_CALL(*fetcher_, Fetch("http-client-response"))
      .WillOnce(::testing::Return(R"(
    local client = require('miet.http.client').get()

    local response, err = client:send('HEAD', 'http://localhost:80/v1/hello')
    if err ~= nil then
      error('Can\'t send request: ' .. err)
    end

    if response['status'] ~= 201 then
      error('Incorrect status')
    end

    if response['headers']['Content-Type'] ~= 'text/plain; charset=utf-8' then
      error('Incorrect headers')
    end

    if response['body'] ~= 'Hello, world!' then
      error('Incorrect body')
    end
  )"));

  EXPECT_CALL(*httpClient_, Send(::testing::_))
      .WillOnce(
          [&]([[maybe_unused]] const http::Request& request) -> http::Response {
            http::Response response;
            response.SetStatus(http::Response::HttpStatus::kCreated);
            const auto headers =
                std::make_shared<userver::http::headers::HeaderMap>();
            headers->emplace(
                userver::http::headers::kContentType,
                userver::http::content_type::kTextPlain.ToString());
            response.SetHeaders(headers);
            response.SetBody("Hello, world!");
            return response;
          });

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("http-client-response", context));
}

UTEST_F(TestLuaExecutor, HttpClientSendAliases) {
  EXPECT_CALL(*fetcher_, Fetch("http-client-send-aliases"))
      .WillOnce(::testing::Return(R"(
    local client = require('miet.http.client').get()

    print(type(client))
    print(client.get)
    local response, err = client:get('http://localhost:80/v1/hello')
    if err ~= nil then
      error('Can\'t send GET request: ' .. err)
    end

    response, err = client:post('http://localhost:80/v1/hello', {
        body = 'Hello, world!'
    })
    if err ~= nil then
      error('Can\'t send POST request: ' .. err)
    end
  )"));

  std::shared_ptr<http::Request::HttpMethod> getMethod = nullptr;
  std::shared_ptr<http::Request::HttpMethod> postMethod = nullptr;

  EXPECT_CALL(*httpClient_, Send(::testing::_))
      .Times(2)
      .WillRepeatedly([&](const http::Request& request) -> http::Response {
        if (!getMethod) {
          getMethod =
              std::make_shared<http::Request::HttpMethod>(request.GetMethod());
        } else {
          postMethod =
              std::make_shared<http::Request::HttpMethod>(request.GetMethod());
        }
        return {};
      });

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("http-client-send-aliases", context));

  ASSERT_EQ(*getMethod, http::Request::HttpMethod::kGet);
  ASSERT_EQ(*postMethod, http::Request::HttpMethod::kPost);
}
