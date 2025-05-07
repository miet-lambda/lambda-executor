#include <miet/lambda/project/storage.hpp>

#include <fmt/format.h>

namespace miet::lambda::project {
Storage::Storage(userver::storages::postgres::ClusterPtr cluster) noexcept
    : cluster_(std::move(cluster)) {}

void Storage::Store(engine::ProjectIdType projectId, std::string_view key,
                    std::string_view value) {
  const auto result = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster, R"(
    INSERT INTO projects_kv_storage (project_id, key, value) VALUES ($1, $2, $3)
    ON CONFLICT (project_id, key) DO UPDATE SET value = EXCLUDED.value
  )",
      static_cast<userver::storages::postgres::Bigint>(projectId), key, value);
  if (result.RowsAffected() != 1) {
    throw std::runtime_error(fmt::format("Can't store value (key = {})", key));
  }
}

std::string Storage::Get(engine::ProjectIdType projectId,
                         std::string_view key) const {
  const auto result = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster, R"(
    SELECT value FROM projects_kv_storage
    WHERE project_id = $1 AND key = $2
  )",
      static_cast<userver::storages::postgres::Bigint>(projectId), key);
  if (result.Size() != 1) {
    throw std::runtime_error(
        fmt::format("No value with such key (key = {})", key));
  }
  return result.Back()["value"].As<std::string>();
}
}  // namespace miet::lambda::project
