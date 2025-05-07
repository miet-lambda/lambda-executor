#pragma once

#include <miet/lambda/engine/base/engine.hpp>

#include <userver/components/component_base.hpp>

namespace miet::lambda::components {
class ScriptEngine final : public userver::components::ComponentBase {
 public:
  static constexpr auto kName = "script-engine";

  ScriptEngine(const userver::components::ComponentConfig& config,
               const userver::components::ComponentContext& context);

  [[nodiscard]] engine::CompilerPtr GetCompiler() const noexcept;
  [[nodiscard]] engine::ExecutorPtr GetExecutor() const noexcept;

 private:
  engine::CompilerPtr compiler_ = nullptr;
  engine::ExecutorPtr executor_ = nullptr;
};
}  // namespace miet::lambda::components
