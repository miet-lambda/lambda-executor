#pragma once

#include <miet/lambda/project/base/storage.hpp>

#include <userver/storages/postgres/cluster.hpp>

namespace miet::lambda::project {
class Storage final : public StorageBase {
 public:
  explicit Storage(userver::storages::postgres::ClusterPtr cluster) noexcept;

  void Store(engine::ProjectIdType projectId, std::string_view key,
             std::string_view value) override;
  std::string Get(engine::ProjectIdType projectId,
                  std::string_view key) const override;

 private:
  userver::storages::postgres::ClusterPtr cluster_ = nullptr;
};
}  // namespace miet::lambda::project
