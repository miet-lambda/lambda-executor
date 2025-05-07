#pragma once

#include <miet/lambda/scripts-storage.hpp>

#include <userver/components/component_base.hpp>

namespace miet::lambda::components {
class ScriptsStorageProvider final : public userver::components::ComponentBase {
 public:
  static constexpr auto kName = "scripts-storage-provider";

  ScriptsStorageProvider(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();

  [[nodiscard]] ScriptsStoragePtr GetStorage() const noexcept;

 private:
  ScriptsStoragePtr storage_ = nullptr;
};
}  // namespace miet::lambda::components
