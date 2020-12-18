
#ifndef LUA_CLIENT_H
#define LUA_CLIENT_H

#include <lua.h>

extern const char *luaError[];

typedef struct {
    lua_CFunction func;
    char *name;
} luaCFunc_t;

int luaInit(lua_State **Lua, luaCFunc_t *cfuncs, const char *filename);
int luaQuit(lua_State **Lua);

#endif
