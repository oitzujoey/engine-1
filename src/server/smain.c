
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/common.h"
#include "../common/lua_sandbox.h"
#include "../common/obj.h"
#include "../common/log.h"
#include "../common/file.h"
#include "../common/cfg.h"

luaCFunc_t luaCFunctions[] = {
	{.func = l_puts,                .name = "l_puts"                },
	{.func = l_loadObj,             .name = "l_loadObj"             },
	{.func = l_log_info,            .name = "l_log_info"            },
	{.func = l_log_warning,         .name = "l_log_warning"         },
	{.func = l_log_error,           .name = "l_log_error"           },
	{.func = l_log_critical_error,  .name = "l_log_critical_error"  },
	{.func = NULL,                  .name = NULL        }
};

const cfg_var_t initialConfigVars[] = {
	{.name = "server",      .vector = 0,    .integer = 0,   .string = NULL, .type = none,   .permissions = CFG_VAR_PERMISSION_NONE},
	{.name = "lua_main",    .vector = 0,    .integer = 0,   .string = "",   .type = string, .permissions = CFG_VAR_PERMISSION_READ},
	{.name = "models",      .vector = 0,    .integer = 0,   .string = "",   .type = string, .permissions = CFG_VAR_PERMISSION_READ},
	{.name = "workspace",   .vector = 0,    .integer = 0,   .string = "",   .type = string, .permissions = CFG_VAR_PERMISSION_READ},
	{.name = NULL,          .vector = 0,    .integer = 0,   .string = NULL, .type = none,   .permissions = CFG_VAR_PERMISSION_NONE}
};

int main(int argc, char *argv[]) {
	
	int error = 0;
	lua_State *Lua;
	const char *luafilename = "smain.lua";
	char *luafilepath = NULL;
	cfg_var_t *lua_main_v;
	cfg_var_t *workspace_v;
	string_t tempString;
	
	log_info(__func__, "Starting engine-1 v0.0 (Server)");
	
	string_init(&tempString);
	
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
			cfg_execString(&tempString);
		}
	}
	
	lua_main_v = cfg_findVar("lua_main");
	if (lua_main_v == NULL) {
		log_critical_error(__func__, "\"lua_main\" does not exist.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	workspace_v = cfg_findVar("workspace");
	if (workspace_v == NULL) {
		log_critical_error(__func__, "\"workspace\" does not exist.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}

	/* @TODO: Do proper file path sanitization. */
	luafilepath = malloc((strlen(workspace_v->string) + strlen(lua_main_v->string) + strlen(luafilename) + strlen(".///") + 1) * sizeof(char));
	sprintf(luafilepath, "./%s/%s/%s", workspace_v->string, lua_main_v->string, luafilename);
	
	/* Before we begin, lock the restricted variables. */
	cfg.lock = true;
	
	log_info(__func__, "Executing \"%s\"", luafilepath);
	error = lua_sandbox_init(&Lua, luaCFunctions, luafilepath);
	if (error) {
		log_critical_error(__func__, "Could not start Lua server");
	}
	
	free(luafilepath);
	
	lua_sandbox_quit(&Lua);
	
	error = ERR_OK;
	
	cleanup_l:
	
	cfg_free();
	
	string_free(&tempString);
	
	log_info(__func__, "Server quit");

    return error;
}
