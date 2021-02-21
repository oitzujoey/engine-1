
#ifndef LUA_SANDBOX_H
#define LUA_SANDBOX_H

#include "types.h"

extern const char *luaError[];

int lua_runFunction(lua_State *luaState, const char *functionName, uint32_t timeout);
void lua_sandbox_addFunctions(lua_State **Lua, luaCFunc_t *cfuncs);
int lua_sandbox_init(lua_State **Lua, const char *filename);
void lua_sandbox_quit(lua_State **Lua);

#endif
