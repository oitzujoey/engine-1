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

	error = lua_pcall(luaState, 0, 0, 0);
	if (error) {
		lua_sandbox_handleError(luaState);
		error("Initial run of Lua exited with error %s", luaError[error]);
		lua_error(luaState);
	}

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

	error = lua_pcall(luaState, 0, 0, 0);
	if (error) {
		lua_sandbox_handleError(luaState);
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

// I've noticed this sometimes print out weird junk. I think an error object isn't always pushed, and maybe the C
// function that throws the error is supposed to push an error object on the stack.
int lua_sandbox_handleError(lua_State *l) {
	int e = ERR_OK;
	const char *errorMessage = lua_tostring(l, -1);
	if (errorMessage != NULL) {
		error("Lua error: \"%s\"", lua_tostring(l, -1));
	}
	return e;
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

	// Add user data types.
	char *userDataTypes[] = {
		"sqlite",
		"shader",
	};
	for (size_t i = 0; i < sizeof(userDataTypes)/sizeof(*userDataTypes); i++) {
		(void) luaL_newmetatable(*Lua, userDataTypes[i]);
		(void) lua_pop(*Lua, 1);
	}

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

	error = lua_pcall(*Lua, 0, 0, 0);
	if (error) {
		lua_sandbox_handleError(*Lua);
		error("Initial run of Lua exited with error %s", luaError[error]);
		error = ERR_CRITICAL;
		return ERR_GENERIC;
	}

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
