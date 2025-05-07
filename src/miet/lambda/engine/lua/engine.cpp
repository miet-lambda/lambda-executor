#include <miet/lambda/engine/lua/engine.hpp>

#include <miet/lambda/engine/lua/compiler.hpp>
#include <miet/lambda/engine/lua/executor.hpp>

#include <miet/lambda/engine/params.hpp>

namespace miet::lambda::engine {
ScriptEnginePtr CreateScriptEngine(ExecutionControlParams execCtlParams,
                                   ExtraParams extraParams) {
  return std::make_shared<lua::ScriptEngine>(std::move(execCtlParams),
                                             std::move(extraParams));
}
}  // namespace miet::lambda::engine

namespace miet::lambda::engine::lua {
ScriptEngine::ScriptEngine(ExecutionControlParams execCtlParams,
                           ExtraParams extraParams)
    : execCtlParams_(std::move(execCtlParams)),
      extraParams_(std::move(extraParams)) {}

CompilerPtr ScriptEngine::CreateCompiler() const {
  return std::make_shared<Compiler>();
}

ExecutorPtr ScriptEngine::CreateExecutor() const {
  return std::make_shared<Executor>(
      execCtlParams_, ModulesDeps{.httpClient = extraParams_.httpClient,
                                  .projectStorage = extraParams_.storage});
}
}  // namespace miet::lambda::engine::lua
