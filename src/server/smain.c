
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/common.h"
#include "../common/lua_sandbox.h"
#include "../common/obj.h"
#include "../common/log.h"
#include "../common/file.h"
#include "../common/cfg.h"
#include "../common/vfs.h"

luaCFunc_t luaCFunctions[] = {
	{.func = l_puts,                .name = "l_puts"                },
	{.func = l_loadObj,             .name = "l_loadObj"             },
	{.func = l_log_info,            .name = "l_log_info"            },
	{.func = l_log_warning,         .name = "l_log_warning"         },
	{.func = l_log_error,           .name = "l_log_error"           },
	{.func = l_log_critical_error,  .name = "l_log_critical_error"  },
	{.func = l_vfs_getFileText,     .name = "l_vfs_getFileText"     },
	{.func = NULL,                  .name = NULL                    }
};

const cfg_var_init_t initialConfigVars[] = {
	{.name = "server",      .vector = 0,    .integer = 0,   .string = NULL, .type = none,   .permissions = CFG_VAR_PERMISSION_NONE},
	{.name = "lua_main",    .vector = 0,    .integer = 0,   .string = "",   .type = string, .permissions = CFG_VAR_PERMISSION_READ},
	{.name = "models",      .vector = 0,    .integer = 0,   .string = "",   .type = string, .permissions = CFG_VAR_PERMISSION_READ},
	{.name = "workspace",   .vector = 0,    .integer = 0,   .string = "",   .type = string, .permissions = CFG_VAR_PERMISSION_READ},
	{.name = NULL,          .vector = 0,    .integer = 0,   .string = NULL, .type = none,   .permissions = CFG_VAR_PERMISSION_NONE}
};

int main(int argc, char *argv[]) {
	
	int error = 0;
	lua_State *Lua;
	const char *luaFileName = "smain.lua";
	string_t luaFilePath;
	cfg_var_t *lua_main_v;
	cfg_var_t *workspace_v;
	string_t tempString;
	
	log_info(__func__, "Starting engine-1 v0.0 (Server)");
	
	string_init(&tempString);
	string_init(&luaFilePath);
	
	log_info(__func__, "Initializing server vars");
	error = cfg_initVars(initialConfigVars);
	if (error == ERR_GENERIC) {
		log_critical_error(__func__, "Could not load initial config vars due to bad initialization table.");
		return ERR_GENERIC;
	}
	else if (error == ERR_OUTOFMEMORY) {
		log_critical_error(__func__, "Out of memory.");
		return ERR_OUTOFMEMORY;
	}

	if (file_exists(AUTOEXEC)) {
		log_info(__func__, "Found \""AUTOEXEC"\"");
		cfg_execFile(AUTOEXEC);
	}

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			string_copy_c(&tempString, argv[i]);
			cfg_execString(&tempString, "Console");
		}
	}
	
	workspace_v = cfg_findVar("workspace");
	if (workspace_v == NULL) {
		log_critical_error(__func__, "\"workspace\" does not exist.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	if (!strcmp(workspace_v->string.value, "")) {
		log_critical_error(__func__, "\"workspace\" has not been set.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = vfs_init(&vfs, &workspace_v->string);
	if (error) {
		log_critical_error(__func__, "Could not start VFS");
		goto cleanup_l;
	}

	lua_main_v = cfg_findVar("lua_main");
	if (lua_main_v == NULL) {
		log_critical_error(__func__, "\"lua_main\" does not exist.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	if (!strcmp(lua_main_v->string.value, "")) {
		log_critical_error(__func__, "\"lua_main\" has not been set.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}

	/* @TODO: Do proper file path sanitization. */
	string_copy(&luaFilePath, &workspace_v->string);
	file_concatenatePath(&luaFilePath, &lua_main_v->string);
	file_concatenatePath(&luaFilePath, string_const(luaFileName));
	// string_concatenate(&luaFilePath, &workspace_v->string);
	// luaFilePath = malloc((strlen(workspace_v->string) + strlen(lua_main_v->string) + strlen(luaFileName) + strlen(".///") + 1) * sizeof(char));
	// sprintf(luaFilePath, "%s/%s/%s", workspace_v->string, lua_main_v->string, luaFileName);
	
	/* Before we begin, lock the restricted variables. */
	cfg.lock = true;
	
	log_info(__func__, "Executing \"%s\"", luaFilePath.value);
	error = lua_sandbox_init(&Lua, luaCFunctions, luaFilePath.value);
	if (error) {
		log_critical_error(__func__, "Could not start Lua server");
	}
	
	lua_sandbox_quit(&Lua);
	
	error = ERR_OK;
	
	cleanup_l:
	
	vfs_free(&vfs);
	
	cfg_free();
	
	string_free(&luaFilePath);
	string_free(&tempString);
	
	log_info(__func__, "Server quit");

    return error;
}
