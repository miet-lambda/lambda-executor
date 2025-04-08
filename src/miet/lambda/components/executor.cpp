#include <miet/lambda/components/executor.hpp>

#include <miet/lambda/fetchers/db-fetcher.hpp>
#include <miet/lambda/http-client.hpp>
#include <miet/lambda/key-value-storage.hpp>
#include <miet/lambda/lua/executor.hpp>
#include <miet/lambda/lua/memory-allocator.hpp>
#include <miet/lambda/timeout-checker.hpp>

#include <userver/clients/http/component.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

constexpr auto kLua = "lua";

namespace miet::lambda::components {
Executor::Executor(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context)
    : ComponentBase(config, context) {
  const auto type = config["language-type"].As<std::string>();
  if (type == kLua) {
    const auto cluster =
        context.FindComponent<userver::components::Postgres>("main-db")
            .GetCluster();
    auto fetcher = std::make_shared<DBScriptsFetcher>(cluster);
    auto& nativeClient =
        context.FindComponent<userver::components::HttpClient>()
            .GetHttpClient();
    auto httpClient = std::make_shared<http::Client>(nativeClient);
    auto storage = std::make_shared<KeyValueStorage>(cluster);
    auto checkersFactory = std::make_shared<TimeoutCheckersFactory>();
    auto allocatorsFactory = std::make_shared<lua::MemoryAllocatorsFactory>();
    executor_ = std::make_shared<lua::Executor>(
        lua::ExecutorParams{
            .scriptsFetcher = std::move(fetcher),
            .timeoutCheckersFactory = std::move(checkersFactory),
            .memoryAllocatorsFactory = std::move(allocatorsFactory)},
        lua::LibsDeps{.httpClient = std::move(httpClient),
                      .kvStorage = std::move(storage)});
  } else {
    throw std::runtime_error("Unexpected executor type");
  }
}

userver::yaml_config::Schema Executor::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<ComponentBase>(R"(
    type: object
    description: The executor provider component
    additionalProperties: false
    properties:
      language-type:
        type: string
        description: The language type of the executor
  )");
}

ExecutorPtr Executor::GetExecutor() const noexcept { return executor_; }
}  // namespace miet::lambda::components
