#pragma once

#include <userver/components/component_base.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace miet::lambda::components {
struct InstanceInfo final {
  std::string hostname;
  std::uint16_t port;
};

class InstanceRegistrator final : public userver::components::ComponentBase {
 public:
  static constexpr auto kName = "instance-registrator";

  InstanceRegistrator(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();

  void OnAllComponentsLoaded() override;
  void OnAllComponentsAreStopping() override;

 private:
  InstanceInfo info_;
  userver::storages::postgres::ClusterPtr cluster_;
};
}  // namespace miet::lambda::components
