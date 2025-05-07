#pragma once

#include <miet/lambda/fetchers/base/scripts-fetcher.hpp>

#include <userver/components/component_base.hpp>

namespace miet::lambda::components {
class ScriptsFetcherProvider final : public userver::components::ComponentBase {
 public:
  static constexpr auto kName = "scripts-fetcher-provider";

  ScriptsFetcherProvider(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();

  [[nodiscard]] fetchers::ScriptsFetcherPtr GetFetcher() const noexcept;

 private:
  fetchers::ScriptsFetcherPtr fetcher_ = nullptr;
};
}  // namespace miet::lambda::components
