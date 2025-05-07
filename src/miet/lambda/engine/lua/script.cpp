#include <miet/lambda/engine/lua/script.hpp>

namespace miet::lambda::engine::lua {
Script::Script() noexcept : ScriptBase(ScriptLanguage::Lua) {}

void Script::SetCodeSnippet(LuaCpp::Registry::LuaCodeSnippet&& snippet) {
  snippet_ = std::move(snippet);
}
LuaCpp::Registry::LuaCodeSnippet& Script::GetCodeSnippet() & noexcept {
  return snippet_;
}
}  // namespace miet::lambda::engine::lua
