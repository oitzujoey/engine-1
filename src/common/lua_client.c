
#include <lualib.h>
#include <lauxlib.h>
#include "lua_client.h"
#include "../common/common.h"

const char *luaError[] = {
    "LUA_OK",
    "LUA_YIELD",
    "LUA_ERRRUN",
    "LUA_ERRSYNTAX",
    "LUA_ERRMEM",
    "LUA_ERRERR"
};

static const luaL_Reg luaLibs[] = {
    // {LUA_GNAME, luaopen_base},
    {NULL, NULL}
};

int luaInit(lua_State **Lua, luaCFunc_t *cfuncs, const char *filename) {
    
    int error = 0;
    int i;
    
    *Lua = luaL_newstate();
    /* Only include this if you are insane. */
    /* luaL_openlibs(Lua); */
    
    const luaL_Reg *lib;
    for (lib = luaLibs; lib->func; lib++) {
        luaL_requiref(*Lua, lib->name, lib->func, 1);
        lua_pop(*Lua, 1);
    }

    lua_pushnil(*Lua);
    
    for (i = 0; cfuncs[i].name != NULL; i++) {
        lua_pushcfunction(*Lua, cfuncs[i].func);
        lua_setglobal(*Lua, cfuncs[i].name);
    }
    
    error = luaL_loadfile(*Lua, filename);
    if (error) {
        fprintf(stderr, "Error: Could not load lua file %s due to error %s\n", filename, luaError[error]);
        return 1;
    }
    
    error = lua_pcall(*Lua, 0, 0, 0);
    if (error) {
        fprintf(stderr, "Error: Lua script %s exited with error %s\n", filename, luaError[error]);
        return 1;
    }
    
}

int luaQuit(lua_State **Lua) {
    lua_close(*Lua);
}
