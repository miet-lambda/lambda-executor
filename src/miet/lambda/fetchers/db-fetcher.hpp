#pragma once

#include <miet/lambda/base/scripts-fetcher.hpp>

#include <userver/storages/postgres/cluster.hpp>

namespace miet::lambda {
class DBScriptsFetcher final : public ScriptsFetcherBase {
 public:
  explicit DBScriptsFetcher(
      userver::storages::postgres::ClusterPtr cluster) noexcept;

  ScriptInfo Fetch(std::string_view id) override;

 private:
  userver::storages::postgres::ClusterPtr cluster_ = nullptr;
};
}  // namespace miet::lambda
