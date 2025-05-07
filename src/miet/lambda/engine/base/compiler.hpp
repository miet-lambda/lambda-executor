#pragma once

#include <miet/lambda/engine/base/script.hpp>

namespace miet::lambda::engine {
class CompilerBase {
 public:
  [[nodiscard]] virtual ScriptPtr Compile(std::string_view code) const = 0;
};

using CompilerPtr = std::shared_ptr<CompilerBase>;
}  // namespace miet::lambda::engine
