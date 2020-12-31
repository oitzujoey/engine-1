
#ifndef LUA_SANDBOX_H
#define LUA_SANDBOX_H

#include <lua.h>
#include "str.h"

extern const char *luaError[];

typedef struct {
    lua_CFunction func;
    char *name;
} luaCFunc_t;

int lua_sandbox_init(lua_State **Lua, luaCFunc_t *cfuncs, const char *filename);
void lua_sandbox_quit(lua_State **Lua);

#endif
