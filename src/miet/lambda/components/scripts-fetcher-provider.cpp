#include <miet/lambda/components/scripts-fetcher-provider.hpp>

#include <miet/lambda/fetchers/db-scripts-fetcher.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>

#include <userver/storages/postgres/component.hpp>

#include <userver/yaml_config/merge_schemas.hpp>

constexpr auto kDBFetcher = "db";

namespace miet::lambda::components {
ScriptsFetcherProvider::ScriptsFetcherProvider(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context) {
  const auto type = config["fetcher-type"].As<std::string>();
  if (type == kDBFetcher) {
    auto cluster =
        context.FindComponent<userver::components::Postgres>("main-db")
            .GetCluster();
    fetcher_ = std::make_shared<fetchers::DBScriptsFetcher>(std::move(cluster));
  } else {
    throw std::invalid_argument(
        fmt::format("Unknown scripts fetcher type (type = {})", type));
  }
}

userver::yaml_config::Schema ScriptsFetcherProvider::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<ComponentBase>(R"(
    type: object
    description: The scripts fetcher provider component
    additionalProperties: false
    properties:
      fetcher-type:
        type: string
        description: The type of fetcher
  )");
}

fetchers::ScriptsFetcherPtr ScriptsFetcherProvider::GetFetcher()
    const noexcept {
  return fetcher_;
}
}  // namespace miet::lambda::components
