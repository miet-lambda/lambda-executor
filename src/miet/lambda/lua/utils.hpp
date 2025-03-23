#pragma once

#include <userver/utils/assert.hpp>

#include <LuaCpp/Engine/LuaType.hpp>

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
}  // namespace miet::lambda::lua::utils
