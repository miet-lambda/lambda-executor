#include <lauxlib.h>

extern "C" int lua_get_http_context(lua_State* L) {
  const int n = lua_gettop(L);
  if (n != 0) {
    return luaL_error(L, "Invalid parameters count (expected 0, actual = %d)",
                      n);
  }
  lua_getglobal(L, "miet_http_context");
  return 1;
}

extern "C" int luaopen_miet_http_context(lua_State* L) {
  luaL_Reg reg[] = {{"get", lua_get_http_context}, {NULL, NULL}};
  luaL_newlib(L, reg);
  return 1;
}
