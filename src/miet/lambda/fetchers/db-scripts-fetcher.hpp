#pragma once

#include <miet/lambda/fetchers/base/scripts-fetcher.hpp>

#include <userver/storages/postgres/cluster.hpp>

namespace miet::lambda::fetchers {
class DBScriptsFetcher final : public ScriptsFetcherBase {
 public:
  explicit DBScriptsFetcher(
      userver::storages::postgres::ClusterPtr cluster) noexcept;

  ScriptInfo Fetch(engine::ScriptIdType id) override;

 private:
  const userver::storages::postgres::ClusterPtr cluster_ = nullptr;
};
}  // namespace miet::lambda::fetchers
