#include <miet/lambda/lua/http-client.hpp>

#include <miet/lambda/lua/utils.hpp>

#include <userver/logging/log.hpp>

namespace miet::lambda::lua {
namespace {
class HttpClientMethod final : public LuaCpp::LuaMetaObject {
 public:
  explicit HttpClientMethod(HttpClientPtr httpClient)
      : httpClient_(std::move(httpClient)) {}

  int Execute(LuaCpp::Engine::LuaState& L) override {
    LOG_DEBUG() << "Start client:send method";
    const auto n = lua_gettop(L);
    if (n != 4 && n != 5) {
      return luaL_error(L,
                        "Invalid arguments count (expected 2 or 3 (method, "
                        "url, [params]), provided %d)",
                        n - 2);
    }
    /** @note Check 'method' type */
    if (!lua_isstring(L, 3)) {
      return luaL_error(L, "Expected a string type of 'method'");
    }
    /** @note Check 'url' type */
    if (!lua_isstring(L, 4)) {
      return luaL_error(L, "Expected a string type of 'url'");
    }
    /** @note Check additional parameters type (query params, headers, body) */
    if (n == 5 && !lua_isnil(L, 5)) {
      if (!lua_istable(L, 5)) {
        return luaL_error(L, "Expected a table type of additional params");
      }
    }
    const std::string_view method = lua_tostring(L, 3);
    const std::string_view url = lua_tostring(L, 4);

    http::Request request;
    request.SetMethod(userver::server::http::HttpMethodFromString(method));
    request.SetUrl(url.data());

    if (n == 5 && !lua_isnil(L, 5)) {
      const auto errorCodeOpt = PopulateAdditionalParams(L, request);
      if (errorCodeOpt) {
        return errorCodeOpt.value();
      }
    }

    lua_settop(L, 0);

    http::Response response;
    try {
      LOG_DEBUG() << "Sending "
                  << userver::server::http::ToString(request.GetMethod())
                  << " request to " << request.GetUrl();
      response = httpClient_->Send(request);
      LOG_DEBUG() << "Sended "
                  << userver::server::http::ToString(request.GetMethod())
                  << " request to " << request.GetUrl();
    } catch (const std::exception& ex) {
      LOG_ERROR() << "Can't send "
                  << userver::server::http::ToString(request.GetMethod())
                  << " request to " << request.GetUrl();
      lua_pushnil(L);
      lua_pushstring(L, ex.what());
      return 2;
    }

    LOG_DEBUG() << "Bilding lua response...";
    BuildLuaResponse(L, response);
    LOG_DEBUG() << "Lua response is built";

    lua_pushnil(L);
    LOG_DEBUG() << "Finish client:send method";
    return 2;
  }

  std::optional<int> PopulateAdditionalParams(LuaCpp::Engine::LuaState& L,
                                              http::Request& request) {
    const auto query =
        std::make_shared<std::unordered_map<std::string, std::string>>();
    {
      lua_pushstring(L, "query");
      const auto type = lua_gettable(L, 5);
      if (type != LUA_TNIL) {
        const auto errorCodeOpt = PopulateQueryParams(L, type, *query);
        if (errorCodeOpt) {
          return errorCodeOpt.value();
        }
      }
    }
    lua_settop(L, 5);
    const auto headers = std::make_shared<userver::http::headers::HeaderMap>();
    {
      lua_pushstring(L, "headers");
      const auto type = lua_gettable(L, 5);
      if (type != LUA_TNIL) {
        const auto errorCodeOpt = PopulateHeaders(L, type, *headers);
        if (errorCodeOpt) {
          return errorCodeOpt.value();
        }
      }
    }
    lua_settop(L, 5);
    std::string body;
    {
      lua_pushstring(L, "body");
      const auto type = lua_gettable(L, 5);
      if (type != LUA_TNIL) {
        const auto errorCodeOpt = PopulateBody(L, type, body);
        if (errorCodeOpt) {
          return errorCodeOpt.value();
        }
      }
    }
    request.SetQueryParams(query);
    request.SetHeaders(headers);
    request.SetBody(std::move(body));
    return std::nullopt;
  }

  std::optional<int> PopulateQueryParams(
      LuaCpp::Engine::LuaState& L, std::int32_t luaType,
      std::unordered_map<std::string, std::string>& query) {
    if (luaType != LUA_TTABLE) {
      return luaL_error(L, "Expected a table type of query");
    }
    lua_pushnil(L);
    while (lua_next(L, 6) != 0) {
      if (!lua_isstring(L, -2) || !lua_isstring(L, -1)) {
        return luaL_error(
            L, "Expected a string type of key and value for query parameter");
      }
      const std::string_view key = lua_tostring(L, -2);
      const std::string_view value = lua_tostring(L, -1);
      query.emplace(key.data(), value.data());
      lua_pop(L, 1);
    }
    return std::nullopt;
  }

  std::optional<int> PopulateHeaders(
      LuaCpp::Engine::LuaState& L, std::int32_t luaType,
      userver::http::headers::HeaderMap& headers) {
    if (luaType != LUA_TTABLE) {
      return luaL_error(L, "Expected a table type of headers");
    }
    lua_pushnil(L);
    while (lua_next(L, 6) != 0) {
      if (!lua_isstring(L, -2) || !lua_isstring(L, -1)) {
        return luaL_error(
            L, "Expected a string type of key and value for headers");
      }
      const std::string_view key = lua_tostring(L, -2);
      const std::string_view value = lua_tostring(L, -1);
      headers.emplace(key, value.data());
      lua_pop(L, 1);
    }
    return std::nullopt;
  }

  std::optional<int> PopulateBody(LuaCpp::Engine::LuaState& L,
                                  std::int32_t luaType, std::string& body) {
    if (luaType != LUA_TSTRING && luaType != LUA_TNUMBER &&
        luaType != LUA_TTABLE && luaType != LUA_TBOOLEAN) {
      return luaL_error(L,
                        "One of these types is expected for the body (string, "
                        "number, table, boolean), actual = %s",
                        lua_typename(L, luaType));
    }
    try {
      body = utils::Serializer::ToString(L);
    } catch (const std::exception& ex) {
      return luaL_error(L, ex.what());
    }
    return std::nullopt;
  }

  void BuildLuaResponse(LuaCpp::Engine::LuaState& L,
                        const http::Response& response) {
    lua_newtable(L);

    lua_pushinteger(L, static_cast<int>(response.GetStatus()));
    lua_setfield(L, -2, "status");

    if (response.GetHeaders()) {
      lua_newtable(L);
      for (const auto& [header, value] : *response.GetHeaders()) {
        lua_pushstring(L, value.data());
        lua_setfield(L, -2, header.data());
      }
      lua_setfield(L, -2, "headers");
    }

    lua_pushstring(L, response.GetBody().data());
    lua_setfield(L, -2, "body");
  }

 private:
  HttpClientPtr httpClient_ = nullptr;
};
}  // namespace

HttpClient::HttpClient(HttpClientPtr httpClient) noexcept
    : httpClient_(std::move(httpClient)) {
  getClientMethod_ = std::make_shared<HttpClientMethod>(httpClient_);
}

std::shared_ptr<LuaCpp::Engine::LuaType> HttpClient::getValue(
    std::string& key) {
  if (key == "send") {
    return getClientMethod_;
  }
  return std::make_shared<LuaCpp::Engine::LuaTNil>();
}
}  // namespace miet::lambda::lua
