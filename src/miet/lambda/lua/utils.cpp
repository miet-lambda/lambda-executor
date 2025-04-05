#include <miet/lambda/lua/utils.hpp>

#include <fmt/format.h>

namespace miet::lambda::lua::utils {
std::string Serializer::ToString(LuaCpp::Engine::LuaState& L,
                                 std::optional<std::size_t> idx) {
  const auto value = ToValue(L, idx).ExtractValue();
  if (value.IsString()) {
    return value.As<std::string>();
  }
  return userver::formats::json::ToString(value);
}

userver::formats::json::ValueBuilder Serializer::ToValue(
    LuaCpp::Engine::LuaState& L, std::optional<std::size_t> idx) {
  const auto type = lua_type(L, idx.value_or(-1));
  switch (type) {
    case LUA_TSTRING:
      return LuaStringToValue(L, idx);
    case LUA_TNUMBER:
      return LuaNumberToValue(L, idx);
    case LUA_TBOOLEAN:
      return LuaBooleanToValue(L, idx);
    case LUA_TTABLE:
      return LuaTableToValue(L, idx);
    default:
      throw std::runtime_error(
          fmt::format("Can't serialize lua object with such type (type = {})",
                      lua_typename(L, type)));
  }
  UINVARIANT(false, "How ? (-_-)");
  return {};
}

userver::formats::json::ValueBuilder Serializer::LuaStringToValue(
    LuaCpp::Engine::LuaState& L, std::optional<std::size_t> idx) {
  return lua_tostring(L, idx.value_or(-1));
}

userver::formats::json::ValueBuilder Serializer::LuaNumberToValue(
    LuaCpp::Engine::LuaState& L, std::optional<std::size_t> idx) {
  return lua_tonumber(L, idx.value_or(-1));
}

userver::formats::json::ValueBuilder Serializer::LuaBooleanToValue(
    LuaCpp::Engine::LuaState& L, std::optional<std::size_t> idx) {
  return static_cast<bool>(lua_toboolean(L, idx.value_or(-1)));
}

userver::formats::json::ValueBuilder Serializer::LuaTableToValue(
    LuaCpp::Engine::LuaState& L, std::optional<std::size_t> idx) {
  userver::formats::json::ValueBuilder builder;
  lua_pushnil(L);
  while (lua_next(L, idx.value_or(-2)) != 0) {
    const auto type = lua_type(L, -2);
    if (type != LUA_TSTRING && type != LUA_TNUMBER) {
      lua_pop(L, 3);
      throw std::runtime_error(fmt::format(
          "The table key must have a string or number type (actual type = {})",
          lua_typename(L, type)));
    }
    userver::formats::json::ValueBuilder value;
    try {
      value = ToValue(L, std::nullopt);
    } catch (...) {
      lua_pop(L, 3);
      throw;
    }
    if (type == LUA_TSTRING) {
      builder.EmplaceNocheck(lua_tostring(L, -2), std::move(value));
    } else {
      builder.PushBack(std::move(value));
    }
    lua_pop(L, 1);
  }
  return builder;
}
}  // namespace miet::lambda::lua::utils
