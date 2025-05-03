#include <miet/lambda/lua/executor.hpp>

#include <miet/lambda/exceptions.hpp>

#include <miet/lambda/testutils/mocks/http-client.hpp>
#include <miet/lambda/testutils/mocks/key-value-storage.hpp>
#include <miet/lambda/testutils/mocks/memory-allocator.hpp>
#include <miet/lambda/testutils/mocks/scripts-fetcher.hpp>
#include <miet/lambda/testutils/mocks/timeout-checker.hpp>
#include <miet/lambda/testutils/utils.hpp>

#include <userver/formats/json/schema.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/http/content_type.hpp>
#include <userver/utest/utest.hpp>

using namespace miet::lambda;

class TestLuaExecutorBase : public testing::Test {
 protected:
  static constexpr auto kTestProjectId = 1;

  void SetUp() override {
    fetcher_ = std::make_shared<ScriptsFetcherMock>();
    checker_ = std::make_shared<TimeoutCheckerMock>();
    allocator_ = std::make_shared<MemoryAllocatorMock>();
    checkersFactory_ = std::make_shared<TimeoutCheckersFactoryMock>();
    allocatorsFactory_ = std::make_shared<MemoryAllocatorsFactoryMock>();
    httpClient_ = std::make_shared<HttpClientMock>();
    kvStorage_ = std::make_shared<KeyValueStorageMock>();
    executor_ = std::make_shared<lua::Executor>(
        lua::ExecutorParams{.scriptsFetcher = fetcher_,
                            .timeoutCheckersFactory = checkersFactory_,
                            .memoryAllocatorsFactory = allocatorsFactory_},
        lua::LibsDeps{.httpClient = httpClient_, .kvStorage = kvStorage_});

    EXPECT_CALL(*checker_, Start()).WillRepeatedly(::testing::DoDefault());
    EXPECT_CALL(*checker_, IsExpired())
        .WillRepeatedly(::testing::Return(false));

    EXPECT_CALL(*checkersFactory_, CreateChecker(::testing::_))
        .WillRepeatedly(::testing::Return(checker_));

    EXPECT_CALL(*allocator_, Free(::testing::_, ::testing::_))
        .WillRepeatedly([](void* ptr, std::int32_t) -> void { free(ptr); });
    EXPECT_CALL(*allocator_, Realloc(::testing::_, ::testing::_, ::testing::_))
        .WillRepeatedly(
            [](void* ptr, std::int32_t, std::int32_t size) -> void* {
              return realloc(ptr, size);
            });

    EXPECT_CALL(*allocatorsFactory_, CreateAllocator())
        .WillRepeatedly(::testing::Return(allocator_));
  }

 protected:
  std::shared_ptr<lua::Executor> executor_ = nullptr;

  std::shared_ptr<ScriptsFetcherMock> fetcher_ = nullptr;
  std::shared_ptr<TimeoutCheckerMock> checker_ = nullptr;
  std::shared_ptr<MemoryAllocatorMock> allocator_ = nullptr;
  std::shared_ptr<TimeoutCheckersFactoryMock> checkersFactory_ = nullptr;
  std::shared_ptr<MemoryAllocatorsFactoryMock> allocatorsFactory_ = nullptr;
  std::shared_ptr<HttpClientMock> httpClient_ = nullptr;
  std::shared_ptr<KeyValueStorageMock> kvStorage_ = nullptr;
};

class TestLuaExecutor : public TestLuaExecutorBase {
 public:
};

UTEST_F(TestLuaExecutor, WithoutContext) {
  EXPECT_CALL(*fetcher_, Fetch("without-context"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
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
  )"}));

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("without-context", context));
}

UTEST_F(TestLuaExecutor, RunCached) {
  EXPECT_CALL(*fetcher_, Fetch("run-cached"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
    --- some big script
  )"}));

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("run-cached", context));
  ASSERT_NO_THROW(executor_->Execute("run-cached", context));
}

UTEST_F(TestLuaExecutor, JsonEncodeDecode) {
  EXPECT_CALL(*fetcher_, Fetch("json-encode-decode"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
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
  )"}));

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("json-encode-decode", context));
}

UTEST_F(TestLuaExecutor, AccessToInternalVariables) {
  EXPECT_CALL(*fetcher_, Fetch("access-to-internal-variable"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
      if miet_execution_timeout_checker ~= nil then -- This variable should not be accessed
          error('Variable "miet_execution_timeout_checker" is not nil')
      end
      if miet_http_context == nil then
          error('Variable "miet_http_context" is nil')
      end
      if miet_http_client == nil then
          error('Variable "miet_http_client" is nil')
      end
      if miet_kv_storage == nil then
          error('Variable "miet_kv_storage" is nil')
      end
  )"}));

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("access-to-internal-variable", context));
}

UTEST_F(TestLuaExecutor, IncomingRequest) {
  EXPECT_CALL(*fetcher_, Fetch("incoming-request"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
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
  )"}));

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
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
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
  )"}));

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
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
    local client = require('miet.http.client').get()
    local context = require('miet.http.context').get()

    local outgoing_response = context:response()

    local response, err = client:send('GET', 'http://localhost:80/v1/error')
    if response ~= nil or err == nil then
      error('Expected send error')
    end

    outgoing_response['headers'] = {
      message = err
    }
  )"}));

  EXPECT_CALL(*httpClient_, Send(::testing::_))
      .WillOnce(::testing::Throw(std::runtime_error(kSendErrorMessage)));

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("http-client-send-error", context));

  ASSERT_TRUE(context->GetResponse().GetHeaders()->contains(kMessageHeader));
  ASSERT_EQ(context->GetResponse().GetHeaders()->at(kMessageHeader),
            kSendErrorMessage);
}

UTEST_F(TestLuaExecutor, HttpClientRequest) {
  EXPECT_CALL(*fetcher_, Fetch("http-client-request"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
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
  )"}));

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
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
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
  )"}));

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
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
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
  )"}));

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
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
    local client = require('miet.http.client').get()

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
  )"}));

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

UTEST_F(TestLuaExecutor, KvStorageStore) {
  EXPECT_CALL(*fetcher_, Fetch("kv-storage-store"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
    local storage = require('miet.kv.storage').get()

    local err = storage:store('name', 'Test')
    if err ~= nil then
      error('Not nil store error: ' .. err)
    end
    
    storage:store('age', 18.5)
    storage:store('is_male', false)
    storage:store('extra', {
      field1 = 'some_data',
      field2 = { 1, 2, 3},
      flag = true
    })
  )"}));

  EXPECT_CALL(*kvStorage_, Store(kTestProjectId, ::testing::_, testing::_))
      .Times(4)
      .WillRepeatedly([]([[maybe_unused]] std::int64_t projectId,
                         std::string_view key, std::string_view value) -> void {
        if (key == "name") {
          ASSERT_EQ(value, "Test");
        } else if (key == "age") {
          ASSERT_EQ(value, "18.5");
        } else if (key == "is_male") {
          ASSERT_EQ(value, "false");
        } else if (key == "extra") {
          const auto extra = userver::formats::json::FromString(value);
          ASSERT_EQ(extra["field1"].As<std::string>(), "some_data");
          ASSERT_EQ(extra["field2"].GetSize(), 3);
          ASSERT_EQ(extra["field2"][0].As<std::uint64_t>(), 1);
          ASSERT_EQ(extra["field2"][1].As<std::uint64_t>(), 2);
          ASSERT_EQ(extra["field2"][2].As<std::uint64_t>(), 3);
          ASSERT_EQ(extra["flag"].As<bool>(), true);
        } else {
          ASSERT_TRUE(false);
        }
      });

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("kv-storage-store", context));
}

UTEST_F(TestLuaExecutor, KvStorageStoreError) {
  constexpr auto kErrorMessage = "Some store error";

  EXPECT_CALL(*fetcher_, Fetch("kv-storage-store-error"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
    local storage = require('miet.kv.storage').get()

    local err = storage:store('message', 'Error')
    if err ~= 'Some store error' then
      error('Expected store error')
    end
  )"}));

  EXPECT_CALL(*kvStorage_, Store(kTestProjectId, ::testing::_, testing::_))
      .WillOnce(::testing::Throw(std::runtime_error(kErrorMessage)));

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("kv-storage-store-error", context));
}

UTEST_F(TestLuaExecutor, KvStorageGet) {
  EXPECT_CALL(*fetcher_, Fetch("kv-storage-get"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
    local storage = require('miet.kv.storage').get()

    local name, err = storage:get('name'):as_string()
    if name ~= 'Test' then
      error('Unexpected \'name\' value: ' .. (err or name))
    end

    local age, err = storage:get('age'):as_number()
    if age ~= 18.5 then
      error('Unexpected \'age\' value: ' .. (err or age))
    end

    local is_male, err = storage:get('is_male'):as_boolean()
    if is_male ~= true then
      error('Unexpected \'is_male\' value: ' .. (err or is_male))
    end

    local extra, err = storage:get('extra'):as_table()
    if extra['field1'] ~= 'some_data' or extra['field2'] ~= 3.14 
       or extra['array'][1] ~= 'first' or extra['array'][2] ~= 'second' then
      error('Unexpected \'extra\' value' .. (err or 'Incorrect table fields'))
    end
  )"}));

  EXPECT_CALL(*kvStorage_, Get(kTestProjectId, ::testing::_))
      .Times(4)
      .WillRepeatedly([]([[maybe_unused]] std::int64_t projectId,
                         std::string_view key) -> std::string {
        if (key == "name") {
          return "Test";
        } else if (key == "age") {
          return "18.5";
        } else if (key == "is_male") {
          return "true";
        } else if (key == "extra") {
          userver::formats::json::ValueBuilder builder;
          builder.EmplaceNocheck("field1", "some_data");
          builder.EmplaceNocheck("field2", 3.14);
          userver::formats::json::ValueBuilder array;
          array.PushBack("first");
          array.PushBack("second");
          builder.EmplaceNocheck("array", array);
          return userver::formats::json::ToString(builder.ExtractValue());
        }
        throw std::runtime_error("No value with such key");
      });

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("kv-storage-get", context));
}

UTEST_F(TestLuaExecutor, KvStorageGetError) {
  constexpr auto kErrorMessage = "Some get error";

  EXPECT_CALL(*fetcher_, Fetch("kv-storage-get-error"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
    local storage = require('miet.kv.storage').get()

    local value, err = storage:get('message'):as_string()
    if value ~= nil or err ~= 'Some get error' then
      error('Expected that value is nil')
    end
  )"}));

  EXPECT_CALL(*kvStorage_, Get(kTestProjectId, ::testing::_))
      .WillOnce(::testing::Throw(std::runtime_error(kErrorMessage)));

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_NO_THROW(executor_->Execute("kv-storage-get-error", context));
}

UTEST_F(TestLuaExecutor, TimeoutError) {
  constexpr auto kChecksLimit = 5;

  EXPECT_CALL(*fetcher_, Fetch("timeout-error"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
      local context = require('miet.http.context').get()

      local response = context:response()

      local iteration = 0
      while true do
        response['body'] = iteration
        iteration = iteration + 1
      end
  )"}));

  std::int64_t currentCheck = 0;

  EXPECT_CALL(*checker_, IsExpired()).WillRepeatedly([&currentCheck]() -> bool {
    if (currentCheck < kChecksLimit) {
      currentCheck++;
      return false;
    }
    return true;
  });

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_THROW(executor_->Execute("timeout-error", context), ExecutionTimout);

  ASSERT_EQ(currentCheck, kChecksLimit);
}

UTEST_F(TestLuaExecutor, OutOfMemoryError) {
  constexpr auto kMaxAllocationsCount = 1000;

  EXPECT_CALL(*fetcher_, Fetch("out-of-memory-error"))
      .WillOnce(::testing::Return(
          ScriptInfo{.projectId = kTestProjectId, .sourceCode = R"(
      local context = require('miet.http.context').get()

      local response = context:response()

      local body = 'spaaaaam'
      for i = 1, 1000 do
        body = body .. 'spaaaaaaaaam'
      end

      response['body'] = body
  )"}));

  std::int64_t currentAllocation = 0;

  EXPECT_CALL(*allocator_, Realloc(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly([&](void* ptr, std::int32_t, std::int32_t size) -> void* {
        if (currentAllocation == kMaxAllocationsCount) {
          return nullptr;
        }
        ++currentAllocation;
        return realloc(ptr, size);
      });

  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_THROW(executor_->Execute("out-of-memory-error", context),
               ExecutionError);

  ASSERT_EQ(currentAllocation, kMaxAllocationsCount);
}

struct TestParams final {
  std::string name;
  std::string script;
};

class TestLuaExecutorParameterized
    : public TestLuaExecutorBase,
      public ::testing::WithParamInterface<TestParams> {
 public:
};

UTEST_P(TestLuaExecutorParameterized, PreventUnsafeScripts) {
  EXPECT_CALL(*fetcher_, Fetch("prevent-unsafe-scripts"))
      .WillOnce(::testing::Return(ScriptInfo{.projectId = kTestProjectId,
                                             .sourceCode = GetParam().script}));
  const auto context = userver::utils::MakeSharedRef<ExecutionContext>(
      http::Request::Default(), http::Response::Default());
  ASSERT_THROW(executor_->Execute("prevent-unsafe-scripts", context),
               ExecutionError);
}

INSTANTIATE_UTEST_SUITE_P(
    UnsafeScripts, TestLuaExecutorParameterized,
    ::testing::Values(
        TestParams{.name = "OsExecute", .script = "os.execute('pwd')"},
        TestParams{.name = "IoOpen", .script = "io.open('secrets.env')"},
        TestParams{.name = "PackageLoadlib",
                   .script = "package.loadlib('glibc.so')"},
        TestParams{.name = "Dofile", .script = "dofile('local-file.lua')"},
        TestParams{.name = "Getfenv", .script = "getfenv()"}),
    [](const ::testing::TestParamInfo<TestParams>& info) -> std::string {
      return info.param.name;
    });

#ifdef NDEBUG
INSTANTIATE_UTEST_SUITE_P(
    IncorrectScripts, TestLuaExecutorParameterized,
    ::testing::Values(
        TestParams{.name = "DebugGetregistry", .script = "debug.getregistry()"},
        TestParams{.name = "Error", .script = "error('some error')"},
        TestParams{.name = "Print", .script = "print('password')"}),
    [](const ::testing::TestParamInfo<TestParams>& info) -> std::string {
      return info.param.name;
    });
#endif
