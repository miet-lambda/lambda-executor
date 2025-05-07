#include <miet/lambda/engine/params.hpp>

#include <miet/lambda/engine/stub/engine.hpp>

namespace miet::lambda::engine {
extern ScriptEnginePtr CreateScriptEngine(
    [[maybe_unused]] ExecutionControlParams execCtlParams,
    [[maybe_unused]] ExtraParams extraParams) {
  return std::make_shared<stub::ScriptEngine>();
}
}  // namespace miet::lambda::engine
