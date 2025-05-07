#include <miet/lambda/engine/lua/modules/kv/storage.hpp>

#include <miet/lambda/engine/lua/utils.hpp>

#include <userver/logging/log.hpp>

namespace miet::lambda::engine::lua::modules::kv {
namespace {
class ResultCastMethod final : public LuaCpp::LuaMetaObject {
 public:
  ResultCastMethod(std::shared_ptr<std::string> value, bool isError,
                   utils::Type type) noexcept
      : value_(std::move(value)), isError_(isError), type(type) {}

  int Execute(LuaCpp::Engine::LuaState& L) override {
    const auto n = lua_gettop(L);
    if (n != 2) {
      return luaL_error(L, "Invalid arguments count (expected 0, provided %d)",
                        n - 2);
    }
    lua_settop(L, 0);
    if (isError_) {
      lua_pushnil(L);
      lua_pushstring(L, value_->c_str());
      return 2;
    }
    try {
      utils::Deserializer::FromString(L, type, *value_);
      lua_pushnil(L);
    } catch (const std::exception& ex) {
      lua_pushnil(L);
      lua_pushstring(L, ex.what());
    }
    return 2;
  }

 private:
  std::shared_ptr<std::string> value_;
  bool isError_ = false;
  utils::Type type = utils::Type::String;
};

class StorageResult final : public LuaCpp::LuaMetaObject {
 public:
  StorageResult(std::string value, bool isError = false) noexcept
      : value_(std::make_shared<std::string>(std::move(value))),
        isError_(isError) {
    asStringMethod_ = std::make_shared<ResultCastMethod>(value_, isError_,
                                                         utils::Type::String);
    asNumbergMethod_ = std::make_shared<ResultCastMethod>(value_, isError_,
                                                          utils::Type::Number);
    asBooleanMethod_ = std::make_shared<ResultCastMethod>(value_, isError_,
                                                          utils::Type::Boolean);
    asTableMethod_ = std::make_shared<ResultCastMethod>(value_, isError_,
                                                        utils::Type::Table);
  }

  std::shared_ptr<LuaCpp::Engine::LuaType> getValue(std::string& key) {
    if (key == "as_string") {
      return asStringMethod_;
    } else if (key == "as_number") {
      return asNumbergMethod_;
    } else if (key == "as_boolean") {
      return asBooleanMethod_;
    } else if (key == "as_table") {
      return asTableMethod_;
    }
    return std::make_shared<LuaCpp::Engine::LuaTNil>();
  }

 private:
  std::shared_ptr<std::string> value_;
  bool isError_ = false;
  std::shared_ptr<LuaCpp::Engine::LuaType> asStringMethod_ = nullptr;
  std::shared_ptr<LuaCpp::Engine::LuaType> asNumbergMethod_ = nullptr;
  std::shared_ptr<LuaCpp::Engine::LuaType> asBooleanMethod_ = nullptr;
  std::shared_ptr<LuaCpp::Engine::LuaType> asTableMethod_ = nullptr;
};

class StoreMethod final : public LuaCpp::LuaMetaObject {
 public:
  explicit StoreMethod(std::int64_t projectId,
                       project::StoragePtr kvStorage) noexcept
      : projectId_(projectId), kvStorage_(std::move(kvStorage)) {}

  int Execute(LuaCpp::Engine::LuaState& L) override {
    LOG_DEBUG() << "Start storage:store method";
    const auto n = lua_gettop(L);
    if (n != 4) {
      return luaL_error(
          L, "Invalid arguments count (expected 2 (key, value), provided %d)",
          n - 2);
    }
    if (!lua_isstring(L, 3)) {
      return luaL_error(L, "Expected a string type of 'key'");
    }
    const std::string_view key = lua_tostring(L, 3);
    const auto value = utils::Serializer::ToString(L);
    lua_settop(L, 0);
    try {
      kvStorage_->Store(projectId_, key, value);
    } catch (const std::exception& ex) {
      lua_pushstring(L, ex.what());
      return 1;
    } catch (...) {
      lua_pushstring(L, "Unknow store error");
      return 1;
    }
    lua_pushnil(L);
    LOG_DEBUG() << "Finish storage:store method";
    return 1;
  }

 private:
  std::int64_t projectId_ = 0;
  project::StoragePtr kvStorage_ = nullptr;
};

class GetMethod final : public LuaCpp::LuaMetaObject {
 public:
  explicit GetMethod(std::int64_t projectId,
                     project::StoragePtr kvStorage) noexcept
      : projectId_(projectId), kvStorage_(std::move(kvStorage)) {}

  int Execute(LuaCpp::Engine::LuaState& L) override {
    LOG_DEBUG() << "Start storage:get method";
    const auto n = lua_gettop(L);
    if (n != 3) {
      return luaL_error(
          L, "Invalid arguments count (expected 1 (key), provided %d)", n - 2);
    }
    if (!lua_isstring(L, 3)) {
      return luaL_error(L, "Expected a string type of 'key'");
    }
    const std::string_view key = lua_tostring(L, -1);
    std::string value;
    lua_settop(L, 0);
    try {
      value = kvStorage_->Get(projectId_, key);
    } catch (const std::exception& ex) {
      result_ = std::make_shared<StorageResult>(ex.what(), true);
      result_->PushValue(L);
      return 1;
    } catch (...) {
      result_ = std::make_shared<StorageResult>("Unknow get error", true);
      result_->PushValue(L);
      return 1;
    }
    result_ = std::make_shared<StorageResult>(std::move(value));
    result_->PushValue(L);
    LOG_DEBUG() << "Finish storage:get method";
    return 1;
  }

 private:
  std::int64_t projectId_ = 0;
  project::StoragePtr kvStorage_ = nullptr;
  std::shared_ptr<StorageResult> result_ = nullptr;
};
}  // namespace

Storage::Storage(ProjectIdType projectId,
                 project::StoragePtr kvStorage) noexcept
    : kvStorage_(std::move(kvStorage)) {
  storeMethod_ = std::make_shared<StoreMethod>(projectId, kvStorage_);
  getMethod_ = std::make_shared<GetMethod>(projectId, kvStorage_);
}

std::shared_ptr<LuaCpp::Engine::LuaType> Storage::getValue(std::string& key) {
  if (key == "store") {
    return storeMethod_;
  } else if (key == "get") {
    return getMethod_;
  }
  return std::make_shared<LuaCpp::Engine::LuaTNil>();
}
}  // namespace miet::lambda::engine::lua::modules::kv
