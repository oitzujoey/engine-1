#include "lua_sandbox.h"
#include <string.h>
#include "../physfs-3.0.2/src/physfs.h"
#include "../lua-5.4.8/lualib.h"
#include "../lua-5.4.8/lauxlib.h"
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
	int e = ERR_CRITICAL;
	
	char *fileText = NULL;
	
	int argc = lua_gettop(luaState);
	if (argc != 1) {
		error("Requires 1 argument.", "");
		lua_error(luaState);
		return ERR_GENERIC;
	}
	
	if (!lua_isstring(luaState, 1)) {
		error("Argument is not a string.", "");
		lua_error(luaState);
		return ERR_GENERIC;
	}
	
	const char *filepath = lua_tostring(luaState, 1);

	cfg2_var_t *lua_main_v = cfg2_findVar(CFG_LUA_MAIN);
	if (lua_main_v == NULL) {
		log_critical_error(__func__, "\""CFG_LUA_MAIN"\" does not exist.");
		lua_error(luaState);
		return ERR_GENERIC;
	}
	if (!strcmp(lua_main_v->string, "")) {
		log_critical_error(__func__, "\""CFG_LUA_MAIN"\" has not been set.");
		lua_error(luaState);
		return ERR_GENERIC;
	}

	char *luaFilePath = NULL;
	str2_copyMalloc(&luaFilePath, lua_main_v->string);
	file_concatenatePath(&luaFilePath, filepath);
	file_resolveRelativePaths(luaFilePath);
	
	if (!PHYSFS_exists(luaFilePath)) {
		error("File \"%s\" does not exist", luaFilePath);
		MEMORY_FREE(&luaFilePath);
		lua_error(luaState);
		return ERR_GENERIC;
	}
	
	e = vfs_getFileText(&fileText, luaFilePath);
	if (e) {
		error("Could not read text from file \"%s\".", luaFilePath);
		e = ERR_GENERIC;
		MEMORY_FREE(&luaFilePath);
		lua_error(luaState);
		return ERR_GENERIC;
	}
	
	e = luaL_loadstring(luaState, fileText);
	// e = luaL_loadfile(*Lua, filename);
	if (e) {
		error("Could not load lua file %s due to error %s", luaFilePath, luaError[e]);
		MEMORY_FREE(&fileText);
		MEMORY_FREE(&luaFilePath);
		lua_error(luaState);
		return ERR_GENERIC;
	}
	
	// Do an initial run of the chunk.

	info("Loading %s", luaFilePath);

	e = lua_pcall(luaState, 0, 0, 0);
	if (e) {
		lua_sandbox_handleError(luaState);
		error("Initial run of Lua exited with error %s", luaError[e]);
		MEMORY_FREE(&fileText);
		MEMORY_FREE(&luaFilePath);
		lua_error(luaState);
		return ERR_GENERIC;
	}

	MEMORY_FREE(&fileText);
	// Freeing this variable causes an error on Windows!
#	ifndef WINDOWS
	free(luaFilePath);
#	endif
	luaFilePath = 0;

	return 0;
}



int lua_runFunction(lua_State *luaState, const char *functionName, uint32_t timeout) {
	int error = ERR_CRITICAL;

	error = lua_getglobal(luaState, functionName);
	if (error != LUA_TFUNCTION) {
		error("Lua file does not contain \"%s\" function.", functionName);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}

	error = lua_pcall(luaState, 0, 0, 0);
	if (error) {
		lua_sandbox_handleError(luaState);
		error("Lua function \"%s\" exited with error %s", functionName, luaError[error]);
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
	error("Lua crashed!", "");
	const char *errorMessage = lua_tostring(l, -1);
	if (errorMessage != NULL) {
		error("Lua error: \"%s\"", lua_tostring(l, -1));
	}
	return e;
}

int lua_sandbox_init(lua_State **Lua, const char *filename) {
	int e = 0;
	
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
		error("File \"%s\" does not exist", filename, luaError[e]);
		return ERR_GENERIC;
	}
	
	e = vfs_getFileText(&fileText, filename);
	if (e) {
		error("Could not open lua file \"%s\".", filename);
		e = ERR_GENERIC;
		goto cleanup_l;
	}
	
	e = luaL_loadstring(*Lua, fileText);
	// e = luaL_loadfile(*Lua, filename);
	if (e) {
		error("Could not load lua file %s due to error %s", filename, luaError[e]);
		return ERR_GENERIC;
	}
	
	// Do an initial run of the chunk.

	info("Loading %s", filename);

	e = lua_pcall(*Lua, 0, 0, 0);
	if (e) {
		lua_sandbox_handleError(*Lua);
		error("Initial run of Lua exited with error %s", luaError[e]);
		e = ERR_CRITICAL;
		return ERR_GENERIC;
	}

	e = ERR_OK;
	cleanup_l:
	
	MEMORY_FREE(&fileText);
	
	return e;
}

void lua_sandbox_quit(lua_State **Lua) {
	lua_close(*Lua);
}

// int lua_sandbox_sanitizeFilepath(char *filepath) {
	
	
	
//     return 0;
// }
