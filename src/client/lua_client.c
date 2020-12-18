
#include <lualib.h>
#include <lauxlib.h>
#include "lua_client.h"
#include "../common/common.h"
#include "render.h"
#include "input.h"

const char *luaError[] = {
    "LUA_OK",
    "LUA_YIELD",
    "LUA_ERRRUN",
    "LUA_ERRSYNTAX",
    "LUA_ERRMEM",
    "LUA_ERRERR"
};

static const luaL_Reg luaLibs[] = {
    {NULL, NULL}
};

int luaInit(lua_State **Lua, const char *filename) {
    
    int error = 0;
    
    *Lua = luaL_newstate();
    /* Only include this if you are insane. */
    /* luaL_openlibs(Lua); */
    
    const luaL_Reg *lib;
    for (lib = luaLibs; lib->func; lib++) {
        luaL_requiref(*Lua, LUA_GNAME, luaopen_base, 1);
        lua_pop(*Lua, 1);
    }

    lua_pushnil(*Lua);
    lua_setglobal(*Lua, "print");
    
    lua_pushcfunction(*Lua, com_puts);
    lua_setglobal(*Lua, "puts");
    lua_pushcfunction(*Lua, render);
    lua_setglobal(*Lua, "render");
    lua_pushcfunction(*Lua, getInput);
    lua_setglobal(*Lua, "getInput");
    
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
