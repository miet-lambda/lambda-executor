#pragma once

#include <userver/formats/json.hpp>
#include <userver/utils/assert.hpp>

#include <LuaCpp/Engine/LuaType.hpp>

#include <optional>

#include <fmt/format.h>

namespace miet::lambda::lua::utils {
template <typename T>
requires std::is_base_of_v<LuaCpp::Engine::LuaType, T> std::shared_ptr<T>
GetValueStrict(const std::shared_ptr<LuaCpp::Engine::LuaType>& value) {
  UINVARIANT(value, "value is Null");
  auto result = std::dynamic_pointer_cast<T>(value);
  if (!result) {
    throw std::runtime_error(
        fmt::format("Unexpected lua type of variable (name = '{}')",
                    value->getGlobalName()));
  }
  return result;
}

template <typename T>
requires std::is_base_of_v<LuaCpp::Engine::LuaType, T> T* GetValueStrict(
    std::conditional_t<std::is_const_v<T>, const LuaCpp::Engine::LuaType,
                       LuaCpp::Engine::LuaType>* value) {
  UINVARIANT(value, "value is Null");
  auto result = dynamic_cast<T*>(value);
  if (!result) {
    throw std::runtime_error(
        fmt::format("Unexpected lua type of variable (name = '{}')",
                    value->getGlobalName()));
  }
  return result;
}

/**
  * @brief Serialize lua value on the stack to a string
  * @note Supported types:
            - string
            - number
            - boolean
            - table
    @note Table serialized as json */
class Serializer final {
 public:
  /**
   * @param L The lua state
   * @param idx The element index on the stack is passed as nullopt if it is on
   * top */
  static std::string ToString(LuaCpp::Engine::LuaState& L,
                              std::optional<std::size_t> idx = std::nullopt);

 private:
  static userver::formats::json::ValueBuilder ToValue(
      LuaCpp::Engine::LuaState& L, std::optional<std::size_t> idx);

  static userver::formats::json::ValueBuilder LuaStringToValue(
      LuaCpp::Engine::LuaState& L, std::optional<std::size_t> idx);
  static userver::formats::json::ValueBuilder LuaNumberToValue(
      LuaCpp::Engine::LuaState& L, std::optional<std::size_t> idx);
  static userver::formats::json::ValueBuilder LuaBooleanToValue(
      LuaCpp::Engine::LuaState& L, std::optional<std::size_t> idx);
  static userver::formats::json::ValueBuilder LuaTableToValue(
      LuaCpp::Engine::LuaState& L, std::optional<std::size_t> idx);
};

enum class Type : uint8_t { String, Number, Boolean, Table };

/**
 * @brief Deserialize value from string
 *        and push the result onto the Lua stack */
class Deserializer final {
 public:
  static void FromString(LuaCpp::Engine::LuaState& L, Type type,
                         std::string_view value);
};
}  // namespace miet::lambda::lua::utils
