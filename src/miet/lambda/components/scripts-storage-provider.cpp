#include <miet/lambda/components/scripts-storage-provider.hpp>

#include <miet/lambda/components/script-engine.hpp>
#include <miet/lambda/components/scripts-fetcher-provider.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>

#include <userver/yaml_config/merge_schemas.hpp>

namespace miet::lambda::components {
ScriptsStorageProvider::ScriptsStorageProvider(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context) {
  auto fetcher = context.FindComponent<ScriptsFetcherProvider>().GetFetcher();
  auto compiler = context.FindComponent<ScriptEngine>().GetCompiler();
  storage_ = std::make_shared<ScriptsStorage>(
      std::move(fetcher), std::move(compiler),
      config["max-cache-size"].As<std::size_t>());
}

userver::yaml_config::Schema ScriptsStorageProvider::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<ComponentBase>(R"(
    type: object
    description: The scripts storage provider component
    additionalProperties: false
    properties:
      max-cache-size:
        type: integer
        description: The maximum cache in memory scripts
  )");
}

ScriptsStoragePtr ScriptsStorageProvider::GetStorage() const noexcept {
  return storage_;
}
}  // namespace miet::lambda::components
