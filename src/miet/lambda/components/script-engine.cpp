#include <miet/lambda/components/script-engine.hpp>

#include <miet/lambda/engine/fwd.hpp>

#include <miet/lambda/execution/control/memory-limit-checker.hpp>
#include <miet/lambda/execution/control/timeout-checker.hpp>

#include <miet/lambda/http/client.hpp>
#include <miet/lambda/project/storage.hpp>

#include <userver/clients/http/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>

namespace miet::lambda::components {
ScriptEngine::ScriptEngine(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : ComponentBase(config, context) {
  auto cluster = context.FindComponent<userver::components::Postgres>("main-db")
                     .GetCluster();
  auto& client =
      context.FindComponent<userver::components::HttpClient>().GetHttpClient();
  const auto engine = engine::CreateScriptEngine(
      {.memLimitCheckersFactory =
           std::make_shared<execution::control::MemoryLimitCheckersFactory>(),
       .timeoutCheckersFactory =
           std::make_shared<execution::control::TimeoutCheckersFactory>()},
      {.httpClient = std::make_shared<http::Client>(client),
       .storage = std::make_shared<project::Storage>(std::move(cluster))});
  compiler_ = engine->CreateCompiler();
  executor_ = engine->CreateExecutor();
}

engine::CompilerPtr ScriptEngine::GetCompiler() const noexcept {
  return compiler_;
}

engine::ExecutorPtr ScriptEngine::GetExecutor() const noexcept {
  return executor_;
}
}  // namespace miet::lambda::components
