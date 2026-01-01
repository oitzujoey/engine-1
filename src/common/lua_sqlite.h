#ifndef LUA_SQLITE_H
#define LUA_SQLITE_H

#include "../lua-5.4.8/lua.h"

int l_sqlite_open(lua_State *l);
int l_sqlite_exec(lua_State *l);
int l_sqlite_close(lua_State *l);

#endif // LUA_SQLITE_H
