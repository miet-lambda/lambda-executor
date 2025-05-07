#include <miet/lambda/engine/lua/compiler.hpp>

#include <miet/lambda/engine/lua/script.hpp>

namespace miet::lambda::engine::lua {
ScriptPtr Compiler::Compile(std::string_view code) const {
  /** @note First parameter - "name" is unused */
  const auto snippetPtr = impl_.CompileString({}, std::string(code));

  auto script = std::make_shared<Script>();
  script->SetCodeSnippet(std::move(*snippetPtr));
  return script;
}
}  // namespace miet::lambda::engine::lua
