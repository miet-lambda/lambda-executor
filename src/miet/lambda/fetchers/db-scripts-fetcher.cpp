#include <miet/lambda/fetchers/db-scripts-fetcher.hpp>

#include <miet/lambda/exceptions.hpp>

#include <fmt/format.h>

namespace miet::lambda::fetchers {
DBScriptsFetcher::DBScriptsFetcher(
    userver::storages::postgres::ClusterPtr cluster) noexcept
    : cluster_(std::move(cluster)) {}

ScriptInfo DBScriptsFetcher::Fetch(engine::ScriptIdType id) {
  const auto result = cluster_->Execute(
      userver::storages::postgres::ClusterHostTypeFlags{
          userver::storages::postgres::ClusterHostType::kSlaveOrMaster,
      },
      R"(
    SELECT parent_project_id, source_code FROM scripts
    WHERE id = $1
  )",
      static_cast<userver::storages::postgres::Bigint>(id));
  if (result.IsEmpty()) {
    throw NotFoundScriptError(
        fmt::format("No script with such id (id = {})", id));
  }
  return {.id = id,
          .projectId = static_cast<std::size_t>(
              result.Back()["parent_project_id"]
                  .As<userver::storages::postgres::Bigint>()),
          .sourceCode = result.Back()["source_code"].As<std::string>()};
}
}  // namespace miet::lambda::fetchers
