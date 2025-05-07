#include <miet/lambda/engine/lua/utils.hpp>

#include <boost/lexical_cast.hpp>

#include <fmt/format.h>

namespace miet::lambda::engine::lua::utils {
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

static void PushString(LuaCpp::Engine::LuaState& L, std::string_view value) {
  lua_pushstring(L, value.data());
}

static void PushNumber(LuaCpp::Engine::LuaState& L, std::string_view value) {
  lua_pushnumber(L, boost::lexical_cast<double>(value));
}

static void PushBoolean(LuaCpp::Engine::LuaState& L, std::string_view value) {
  lua_pushboolean(L, (value == "true"));
}

static Type GetType(const userver::formats::json::Value& value) {
  if (value.IsBool()) {
    return Type::Boolean;
  } else if (value.IsInt() || value.IsInt64() || value.IsUInt64() ||
             value.IsDouble()) {
    return Type::Number;
  } else if (value.IsString()) {
    return Type::String;
  } else if (value.IsObject() || value.IsArray()) {
    return Type::Table;
  }
  throw std::runtime_error("Unexpected json value type");
}

static void PushValue(LuaCpp::Engine::LuaState& L,
                      const userver::formats::json::Value& value) {
  const auto type = GetType(value);
  try {
    if (value.IsString()) {
      Deserializer::FromString(L, type, value.As<std::string>());
    } else {
      const auto stringValue = userver::formats::json::ToString(value);
      Deserializer::FromString(L, type, stringValue);
    }
  } catch (...) {
    lua_pop(L, 2);
    throw;
  }
}

static void PushTable(LuaCpp::Engine::LuaState& L, std::string_view value) {
  const auto json = userver::formats::json::FromString(value);
  lua_newtable(L);
  if (json.IsObject()) {
    for (const auto& [key, value] : userver::formats::json::Items(json)) {
      lua_pushstring(L, key.c_str());
      PushValue(L, value);
      lua_settable(L, -3);
    }
    return;
  } else if (json.IsArray()) {
    std::size_t i = 1;
    for (const auto& value : json) {
      lua_pushnumber(L, i);
      PushValue(L, value);
      lua_settable(L, -3);
      ++i;
    }
    return;
  }
  lua_pop(L, 1);
  throw std::runtime_error("Value is not a table");
}

void Deserializer::FromString(LuaCpp::Engine::LuaState& L, Type type,
                              std::string_view value) {
  switch (type) {
    case Type::String:
      PushString(L, value);
      break;
    case Type::Number:
      PushNumber(L, value);
      break;
    case Type::Boolean:
      PushBoolean(L, value);
      break;
    case Type::Table:
      PushTable(L, value);
      break;
    default:
      throw std::runtime_error("Unknow lua deserialization type");
  }
}
}  // namespace miet::lambda::engine::lua::utils
