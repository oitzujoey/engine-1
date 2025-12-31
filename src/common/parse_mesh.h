#ifndef PARSE_MESH_H
#define PARSE_MESH_H

#include "../lua-5.4.8/lua.h"

int l_cmsh_load(lua_State *luaState);
int l_rmsh_load(lua_State *luaState);
int l_mesh_load(lua_State *luaState);

#endif // PARSE_MESH_H
