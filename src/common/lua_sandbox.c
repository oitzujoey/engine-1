#include "lua_sandbox.h"
#include <lualib.h>
#include <lauxlib.h>
#include <physfs.h>
#include <string.h>
#include "common.h"
#include "log.h"
#include "file.h"
#include "vfs.h"
#include "cfg2.h"
#include "str2.h"
#include "memory.h"

const char *luaError[] = {
	"LUA_OK",
	"LUA_YIELD",
	"LUA_ERRRUN",
	"LUA_ERRSYNTAX",
	"LUA_ERRMEM",
	"LUA_ERRERR",
	"LUA_ERRFILE"
};

static const luaL_Reg luaLibs[] = {
	// {LUA_GNAME, luaopen_base},
	{NULL, NULL}
};

uint32_t lua_luaTimeout(uint32_t interval, void *param) {
	luaTimeout_t *luaTimeout = param;
	error("Lua function \"%s\" timed out.", luaTimeout->functionName);
	lua_error(luaTimeout->luaState);
	return 0;
}



static int l_lua_sandbox_include(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	char *fileText = NULL;
	
	int argc = lua_gettop(luaState);
	if (argc != 1) {
		error("Requires 1 argument.", "");
		lua_error(luaState);
	}
	
	if (!lua_isstring(luaState, 1)) {
		error("Argument is not a string.", "");
		lua_error(luaState);
	}
	
	const char *filepath = lua_tostring(luaState, 1);
	
	// if (!vfs_isInWorkspace(filepath)) {
	// 	error("File \"%s\" is not in workspace.", filepath);
	// 	lua_error(luaState);
	// }
	
	cfg2_var_t *lua_main_v = cfg2_findVar(CFG_LUA_MAIN);
	if (lua_main_v == NULL) {
		log_critical_error(__func__, "\""CFG_LUA_MAIN"\" does not exist.");
		lua_error(luaState);
	}
	if (!strcmp(lua_main_v->string, "")) {
		log_critical_error(__func__, "\""CFG_LUA_MAIN"\" has not been set.");
		lua_error(luaState);
	}

	/* @TODO: Do proper file path sanitization. */
	char *luaFilePath = NULL;
	str2_copyMalloc(&luaFilePath, lua_main_v->string);
	file_concatenatePath(&luaFilePath, filepath);
	file_resolveRelativePaths(luaFilePath);
	
	if (!PHYSFS_exists(luaFilePath)) {
	// if (!file_exists(luaFilePath)) {
		error("File \"%s\" does not exist", luaFilePath);
		MEMORY_FREE(&luaFilePath);
		lua_error(luaState);
	}
	
	error = vfs_getFileText(&fileText, luaFilePath);
	if (error) {
		error("Could not read text from file \"%s\".", luaFilePath);
		error = ERR_GENERIC;
		MEMORY_FREE(&luaFilePath);
		lua_error(luaState);
	}
	
	error = luaL_loadstring(luaState, fileText);
	// error = luaL_loadfile(*Lua, filename);
	if (error) {
		error("Could not load lua file %s due to error %s", luaFilePath, luaError[error]);
		MEMORY_FREE(&fileText);
		MEMORY_FREE(&luaFilePath);
		lua_error(luaState);
	}
	
	// Do an initial run of the chunk.
	
	luaTimeout_t luaTimeout = {
		.functionName = luaFilePath,
		.luaState = luaState
	};
	
	info("Loading %s", luaFilePath);
	
	SDL_TimerID timerId = SDL_AddTimer(100, lua_luaTimeout, &luaTimeout);
	
	error = lua_pcall(luaState, 0, 0, 0);
	if (error) {
		error("Initial run of Lua exited with error %s", luaError[error]);
		lua_error(luaState);
	}
	
	SDL_RemoveTimer(timerId);
	
	MEMORY_FREE(&fileText);
	MEMORY_FREE(&luaFilePath);
	
	return 0;
}



int lua_runFunction(lua_State *luaState, const char *functionName, uint32_t timeout) {
	int error = ERR_CRITICAL;
	
	luaTimeout_t luaTimeout = {
		.functionName = functionName,
		.luaState = luaState
	};
	
	error = lua_getglobal(luaState, luaTimeout.functionName);
	if (error != LUA_TFUNCTION) {
		error("Lua file does not contain \"%s\" function.", luaTimeout.functionName);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	SDL_TimerID timerId = SDL_AddTimer(timeout, lua_luaTimeout, &luaTimeout);
	
	error = lua_pcall(luaState, 0, 0, 0);
	// lua_call(luaState, 0, 0);
	SDL_RemoveTimer(timerId);
	if (error) {
		error("Lua function \"%s\" exited with error %s", luaTimeout.functionName, luaError[error]);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

void lua_sandbox_addFunctions(lua_State **Lua, luaCFunc_t *cfuncs) {

	int i;
	
	for (i = 0; cfuncs[i].name != NULL; i++) {
		lua_pushcfunction(*Lua, cfuncs[i].func);
		lua_setglobal(*Lua, cfuncs[i].name);
	}
}

int lua_sandbox_init(lua_State **Lua, const char *filename) {
	int error = 0;
	
	char *fileText = NULL;
	
	*Lua = luaL_newstate();
	/* Only include this if you are insane. */
#ifdef DEBUG
	luaL_openlibs(*Lua);
#endif
	
	const luaL_Reg *lib;
	for (lib = luaLibs; lib->func; lib++) {
		luaL_requiref(*Lua, lib->name, lib->func, 1);
		lua_pop(*Lua, 1);
	}

	lua_pushnil(*Lua);
	
	lua_pushcfunction(*Lua, l_lua_sandbox_include);
	lua_setglobal(*Lua, "include");
	
	if (!PHYSFS_exists(filename)) {
		error("File \"%s\" does not exist", filename, luaError[error]);
		return ERR_GENERIC;
	}
	
	error = vfs_getFileText(&fileText, filename);
	if (error) {
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = luaL_loadstring(*Lua, fileText);
	// error = luaL_loadfile(*Lua, filename);
	if (error) {
		error("Could not load lua file %s due to error %s", filename, luaError[error]);
		return ERR_GENERIC;
	}
	
	// Do an initial run of the chunk.
	
	luaTimeout_t luaTimeout = {
		.functionName = filename,
		.luaState = *Lua
	};
	
	info("Loading %s", filename);
	
	SDL_TimerID timerId = SDL_AddTimer(100, lua_luaTimeout, &luaTimeout);
	
	error = lua_pcall(*Lua, 0, 0, 0);
	if (error) {
		log_error(__func__, "Initial run of Lua exited with error %s", luaError[error]);
		error = ERR_CRITICAL;
		return ERR_GENERIC;
	}
	
	SDL_RemoveTimer(timerId);
	
	error = ERR_OK;
	cleanup_l:
	
	MEMORY_FREE(&fileText);
	
	return error;
}

void lua_sandbox_quit(lua_State **Lua) {
	lua_close(*Lua);
}

// int lua_sandbox_sanitizeFilepath(char *filepath) {
	
	
	
//     return 0;
// }
