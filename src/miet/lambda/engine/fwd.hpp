#pragma once

#include <miet/lambda/engine/base/engine.hpp>

#include <miet/lambda/engine/params.hpp>

namespace miet::lambda::engine {
extern ScriptEnginePtr CreateScriptEngine(ExecutionControlParams execCtlParams,
                                          ExtraParams extraParams);
}  // namespace miet::lambda::engine
