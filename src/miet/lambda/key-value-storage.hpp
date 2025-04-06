#pragma once

#include <miet/lambda/base/key-value-storage.hpp>

#include <userver/storages/postgres/cluster.hpp>

namespace miet::lambda {
class KeyValueStorage final : public KeyValueStorageBase {
 public:
  explicit KeyValueStorage(
      userver::storages::postgres::ClusterPtr cluster) noexcept;

  void Store(std::int64_t projectId, std::string_view key,
             std::string_view value) override;
  std::string Get(std::int64_t projectId, std::string_view key) const override;

 private:
  userver::storages::postgres::ClusterPtr cluster_ = nullptr;
};
}  // namespace miet::lambda
