#include <miet/lambda/components/executor.hpp>

#include <miet/lambda/fetchers/db-fetcher.hpp>
#include <miet/lambda/lua/executor.hpp>

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
    auto cluster =
        context.FindComponent<userver::components::Postgres>("main-db")
            .GetCluster();
    auto fetcher = std::make_shared<DBScriptsFetcher>(std::move(cluster));
    executor_ =
        std::make_shared<miet::lambda::lua::Executor>(std::move(fetcher));
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
