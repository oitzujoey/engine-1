
#ifndef LUA_COMMON_H
#define LUA_COMMON_H

#include "types.h"

extern luaCFunc_t luaCommonFunctions[];

int l_common_round(lua_State *l);
int l_common_cos(lua_State *l);
int l_common_sin(lua_State *l);
int l_common_toString(lua_State *luaState);
int l_cfg2_setVariable(lua_State *luaState);
int l_cfg2_setCallback(lua_State *luaState);

int lua_common_printTable(lua_State *luaState);

#endif
