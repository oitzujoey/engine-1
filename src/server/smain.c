
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <SDL2/SDL.h>
#include "../common/common.h"
#include "../common/lua_sandbox.h"
#include "../common/obj.h"
#include "../common/log.h"
#include "../common/file.h"
#include "../common/cfg.h"
#include "../common/vfs.h"
#include "snetwork.h"

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
	{.name = "server",          .vector = 0,    .integer = 0,                   .string = NULL, .type = none,       .permissions = CFG_VAR_PERMISSION_NONE},
	{.name = "lua_main",        .vector = 0,    .integer = 0,                   .string = "",   .type = string,     .permissions = CFG_VAR_PERMISSION_READ},
	{.name = "workspace",       .vector = 0,    .integer = 0,                   .string = "",   .type = string,     .permissions = CFG_VAR_PERMISSION_READ},
	{.name = "server_port",     .vector = 0,    .integer = DEFAULT_PORT_NUMBER, .string = "",   .type = integer,    .permissions = CFG_VAR_PERMISSION_READ},
	{.name = "client_port",     .vector = 0,    .integer = DEFAULT_PORT_NUMBER, .string = "",   .type = integer,    .permissions = CFG_VAR_PERMISSION_READ},
	{.name = NULL,              .vector = 0,    .integer = 0,                   .string = NULL, .type = none,       .permissions = CFG_VAR_PERMISSION_NONE}
};

static int main_init(void) {

	int error = 0;
	
	/*  As of SDL v2.0.14:
		Allocates 220 bytes that SDL_Quit doesn't free.
		I don't think I can do anything about it. */
	error = SDL_Init(SDL_INIT_TIMER);
	if (error != 0) {
		critical_error("SDL_Init returned %s", SDL_GetError());
		return ERR_CRITICAL;
	}
	
	error = snetwork_init();
	if (error) {
		critical_error("Could not initialize network", "");
		return ERR_CRITICAL;
	}

	return ERR_OK;
}

static void main_quit(void) {
	
	snetwork_quit();
	
	SDL_Quit();
}

int main(int argc, char *argv[]) {
	
	int error = 0;
	lua_State *Lua;
	const char *luaFileName = "smain.lua";
	string_t luaFilePath;
	cfg_var_t *lua_main_v;
	cfg_var_t *workspace_v;
	string_t tempString;
	IPaddress ipAddress;
	
	log_info(__func__, "Starting engine-1 v0.0 (Server)");
	
	string_init(&tempString);
	string_init(&luaFilePath);
	
	log_info(__func__, "Initializing server vars");
	error = cfg_initVars(initialConfigVars);
	if (error == ERR_GENERIC) {
		log_critical_error(__func__, "Could not load initial config vars due to bad initialization table.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	else if (error == ERR_OUTOFMEMORY) {
		log_critical_error(__func__, "Out of memory.");
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
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
	
	error = vfs_init(&vfs_g, &workspace_v->string);
	if (error) {
		log_critical_error(__func__, "Could not start VFS");
		goto cleanup_l;
	}
	
	error = main_init();
	if (error) {
		error = ERR_CRITICAL;
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
	
	error = SDLNet_ResolveHost(&ipAddress, "localhost", cfg_findVar("client_port")->integer);
	if (error != 0) {
		critical_error("SDLNet_ResolveHost returned %s", SDL_GetError());
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	l_snetwork_send((Uint8 *) "Hello, world!", strlen("Hello, world!") + 1, ipAddress);
	
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
	
	main_quit();
	
	vfs_free(&vfs_g);
	
	cfg_free();
	
	string_free(&luaFilePath);
	string_free(&tempString);
	
	// I'm leaving this because it works and it's cool.
	info("Server quit (%s)", (char*[4]){"OK", "Error", "Critical error", "Out of memory"}[error]);

    return error;
}
