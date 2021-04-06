
#ifndef LUA_COMMON_H
#define LUA_COMMON_H

#include "types.h"

extern luaCFunc_t luaCommonFunctions[];

int l_common_toString(lua_State *luaState);
int l_cfg2_setVariable(lua_State *luaState);
int l_cfg2_setCallback(lua_State *luaState);

#endif
