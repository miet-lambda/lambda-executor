#pragma once

#include <miet/lambda/base/executor.hpp>

#include <userver/components/component_base.hpp>

namespace miet::lambda::components {
class Executor final : public userver::components::ComponentBase {
 public:
  static constexpr auto kName = "executor";

  Executor(const userver::components::ComponentConfig& config,
           const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();

  ExecutorPtr GetExecutor() const noexcept;

 private:
  ExecutorPtr executor_ = nullptr;
};
}  // namespace miet::lambda::components
