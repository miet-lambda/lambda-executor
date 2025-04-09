#include <miet/lambda/components/instance-registrator.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/pg_types.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace miet::lambda::components {
InstanceRegistrator::InstanceRegistrator(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      cluster_(context.FindComponent<userver::components::Postgres>("main-db")
                   .GetCluster()) {
  info_.hostname = config["hostname"].As<std::string>();
  info_.port = config["port"].As<std::uint16_t>();
}

userver::yaml_config::Schema InstanceRegistrator::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<ComponentBase>(R"(
    type: object
    description: The instance of executor registrator
    additionalProperties: false
    properties:
      hostname:
        type: string
        description: The host on which the instance is running
      port:
        type: string
        description: The port on which the instance is running
  )");
}

void InstanceRegistrator::OnAllComponentsLoaded() {
  cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, R"(
    CREATE TABLE IF NOT EXISTS active_runner_instances (
      ip_address VARCHAR(255) NOT NULL,
      port SMALLINT NOT NULL,
      PRIMARY KEY (ip_address, port)
    )
  )");
  const auto result = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster, R"(
    INSERT INTO active_runner_instances (ip_address, port) VALUES ($1, $2)
    ON CONFLICT (ip_address, port) DO NOTHING
  )",
      info_.hostname, userver::storages::postgres::Smallint(info_.port));
  if (result.RowsAffected() != 1) {
    throw std::runtime_error(
        fmt::format("Can't register instance (hostname = {}, port = {})",
                    info_.hostname, info_.port));
  }
}

void InstanceRegistrator::OnAllComponentsAreStopping() {
  const auto result = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster, R"(
    DELETE FROM active_runner_instances
    WHERE ip_address = $1 AND port = $2
  )",
      info_.hostname, userver::storages::postgres::Smallint(info_.port));
  if (result.RowsAffected() != 1) {
    LOG_CRITICAL() << fmt::format(
        "Can't unregister instance (hostname = {}, port = {})", info_.hostname,
        info_.port);
  }
}
}  // namespace miet::lambda::components
