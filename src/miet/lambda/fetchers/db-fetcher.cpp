#include <miet/lambda/fetchers/db-fetcher.hpp>

#include <miet/lambda/exceptions.hpp>

#include <fmt/format.h>

namespace miet::lambda {
DBScriptsFetcher::DBScriptsFetcher(
    userver::storages::postgres::ClusterPtr cluster) noexcept
    : cluster_(std::move(cluster)) {}

ScriptInfo DBScriptsFetcher::Fetch(std::string_view id) {
  const auto result = cluster_->Execute(
      userver::storages::postgres::ClusterHostTypeFlags{
          userver::storages::postgres::ClusterHostType::kSlaveOrMaster,
      },
      R"(
    SELECT parent_project_id, source_code FROM scripts
    WHERE id = $1
  )",
      std::stoi(id.data()));
  if (result.IsEmpty()) {
    throw NotFoundScriptError(
        fmt::format("No script with such id (id = {})", id));
  }
  return {.projectId = result.Back()["parent_project_id"].As<std::int64_t>(),
          .sourceCode = result.Back()["source_code"].As<std::string>()};
}
}  // namespace miet::lambda
