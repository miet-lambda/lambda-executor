#include <miet/lambda/key-value-storage.hpp>

#include <fmt/format.h>

namespace miet::lambda {
KeyValueStorage::KeyValueStorage(
    userver::storages::postgres::ClusterPtr cluster) noexcept
    : cluster_(std::move(cluster)) {}

void KeyValueStorage::Store(std::int64_t projectId, std::string_view key,
                            std::string_view value) {
  const auto result = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster, R"(
    INSERT INTO projects_kv_storage (project_id, key, value) VALUES ($1, $2, $3)
    ON CONFLICT (project_id, key) DO UPDATE SET value = EXCLUDED.value
  )",
      projectId, key, value);
  if (result.RowsAffected() != 1) {
    throw std::runtime_error(fmt::format("Can't store value (key = {})", key));
  }
}

std::string KeyValueStorage::Get(std::int64_t projectId,
                                 std::string_view key) const {
  const auto result = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster, R"(
    SELECT value FROM projects_kv_storage
    WHERE project_id = $1 AND key = $2
  )",
      projectId, key);
  if (result.Size() != 1) {
    throw std::runtime_error(
        fmt::format("No value with such key (key = {})", key));
  }
  return result.Back()["value"].As<std::string>();
}
}  // namespace miet::lambda
