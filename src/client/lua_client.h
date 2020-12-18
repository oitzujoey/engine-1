
#ifndef LUA_CLIENT_H
#define LUA_CLIENT_H

#include <lua.h>

extern const char *luaError[];

int luaInit(lua_State **Lua, const char *filename);
int luaQuit(lua_State **Lua);

#endif
