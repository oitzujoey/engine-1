
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "../common/common.h"
#include "../common/lua_sandbox.h"
#include "../common/obj.h"
#include "../common/log.h"
#include "../common/file.h"
#include "../common/cfg2.h"
#include "../common/vfs.h"
#include "snetwork.h"
#include "../common/lua_common.h"
#include "../common/entity.h"
#include "../common/network.h"
#include "../common/vector.h"
#include "../common/terminal.h"

static int l_main_housekeeping(lua_State *luaState);
int l_main_checkQuit(lua_State *luaState);

luaCFunc_t luaCFunctions[] = {
	{.func = l_puts,                    .name = "l_puts"                    },
	{.func = l_loadObj,                 .name = "l_loadObj"                 },
	{.func = l_log_info,                .name = "l_log_info"                },
	{.func = l_log_warning,             .name = "l_log_warning"             },
	{.func = l_log_error,               .name = "l_log_error"               },
	{.func = l_log_critical_error,      .name = "l_log_critical_error"      },
	{.func = l_vfs_getFileText,         .name = "l_vfs_getFileText"         },
	{.func = l_obj_loadOoliteDAT,       .name = "l_loadOoliteModel"         },
	{.func = l_common_toString,         .name = "l_toString"                },
	{.func = l_entity_createEntity,     .name = "l_createEntity"            },
	{.func = l_entity_linkChild,        .name = "l_entity_linkChild"        },
	{.func = l_entity_setPosition,      .name = "l_entity_setPosition"      },
	{.func = l_entity_setOrientation,   .name = "l_entity_setOrientation"   },
	{.func = l_main_housekeeping,       .name = "l_main_housekeeping"       },
	{.func = l_main_checkQuit,          .name = "l_checkQuit"               },
	{.func = l_hamiltonProduct,         .name = "l_hamiltonProduct"         },
	{.func = l_quatNormalize,           .name = "l_quatNormalize"           },
	{.func = NULL,                      .name = NULL                        }
};

// const cfg_var_init_t initialConfigVars[] = {
// 	{.name = "server",                  .vector = 0,    .integer = 0,                               .string = NULL, .type = none,       .handle = NULL,                                     .permissions = CFG_VAR_FLAG_NONE},
// 	{.name = "lua_main",                .vector = 0,    .integer = 0,                               .string = "",   .type = string,     .handle = NULL,                                     .permissions = CFG_VAR_FLAG_READ},
// 	{.name = "workspace",               .vector = 0,    .integer = 0,                               .string = "",   .type = string,     .handle = vfs_handle_setWorkspace,                  .permissions = CFG_VAR_FLAG_READ},
// 	{.name = CFG_PORT,                  .vector = 0,    .integer = CFG_PORT_DEFAULT,                .string = "",   .type = integer,    .handle = snetwork_handle_setServerPort,            .permissions = CFG_VAR_FLAG_READ},
// 	{.name = CFG_CONNECTION_TIMEOUT,    .vector = 0,    .integer = CFG_CONNECTION_TIMEOUT_DEFAULT,  .string = "",   .type = integer,    .handle = network_handle_connectionTimeout,         .permissions = CFG_VAR_FLAG_READ},
// 	{.name = CFG_MAX_CLIENTS,           .vector = 0,    .integer = CFG_MAX_CLIENTS_DEFAULT,         .string = "",   .type = integer,    .handle = snetwork_handle_maxClients,               .permissions = CFG_VAR_FLAG_READ},
// 	{.name = CFG_MAX_RECURSION,         .vector = 0,    .integer = CFG_MAX_RECURSION_DEFAULT,       .string = "",   .type = integer,    .handle = cfg_handle_maxRecursion,                  .permissions = CFG_VAR_FLAG_READ},
// 	{.name = CFG_RUN_QUIET,             .vector = 0,    .integer = 0,                               .string = "",   .type = integer,    .handle = NULL,                                     .permissions = CFG_VAR_FLAG_READ},
// 	{.name = CFG_HISTORY_LENGTH,        .vector = 0,    .integer = CFG_HISTORY_LENGTH_DEFAULT,      .string = "",   .type = integer,    .handle = cfg_handle_updateCommandHistoryLength,    .permissions = CFG_VAR_FLAG_READ},
// 	{.name = "enet_message",            .vector = 0,    .integer = 0,                               .string = "",   .type = string,     .handle = snetwork_handle_enetMessage,              .permissions = CFG_VAR_FLAG_WRITE},
// 	{.name = NULL,                      .vector = 0,    .integer = 0,                               .string = NULL, .type = none,       .handle = NULL,                                     .permissions = CFG_VAR_FLAG_NONE}
// };

const cfg2_var_init_t g_serverVarInit[] = {
	// Commands
	// Variables
	{
		.name = "server",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_supervisor,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = NULL
	},
	{
		.name = CFG_PORT,
		.vector = 0,
		.integer = CFG_PORT_DEFAULT,
		.string = "",
		.type = cfg2_var_type_string,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_supervisor,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = snetwork_callback_setServerPort
	},
	{
		.name = NULL,
		.vector = 0,
		.integer = 0,
		.string = NULL,
		.type = 0,
		.permissionRead = 0,
		.permissionWrite = 0,
		.permissionDelete = 0,
		.permissionCallback = 0,
		.callback = NULL
	}
};

int l_main_checkQuit(lua_State *luaState) {
	lua_pushinteger(luaState, g_cfg2.quit);
	return 1;
}

static int l_main_housekeeping(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// char sendString[100];
	// IPaddress ipAddress;
	
	/* Send packet. */
	
	// error = SDLNet_ResolveHost(&ipAddress, "localhost", cfg_findVar("client_port")->integer);
	// if (error != 0) {
	// 	critical_error("SDLNet_ResolveHost returned %s", SDL_GetError());
	// 	error = ERR_CRITICAL;
	// 	goto cleanup_l;
	// }
	
	// strcpy(sendString, "Hello, world!");
	// network_sendReliablePacket(g_serverSocket, ipAddress, (Uint8 *) sendString, strlen(sendString) + 1);
	
	error = snetwork_runEvents();
	if (error) {
		goto cleanup_l;
	}
	
	/* Do terminal stuff */
	
	error = terminal_runTerminalCommand();
	goto cleanup_l;
	
	error = ERR_OK;
	cleanup_l:
	if (error) {
		g_cfg2.quit = true;
	}
	
	SDL_Delay(8);
	
	return 0;
}

static int main_init(int argc, char *argv[], lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	string_t tempString;
	
	string_init(&tempString);
	
	log_info(__func__, "Initializing server vars");
	
	cfg2_init(luaState);
	
	error = cfg2_createVariables(g_serverVarInit);
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
	
	error = cfg2_createVariables(g_commonVarInit);
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
	
	error = terminal_initConsole();
	if (error) {
		goto cleanup_l;
	}

	if (file_exists(AUTOEXEC)) {
		log_info(__func__, "Found \""AUTOEXEC"\"");
		cfg2_execFile(AUTOEXEC, 0);
	}

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			string_copy_c(&tempString, argv[i]);
			error = cfg2_execString(&tempString, "Console", 0);
			if (error == ERR_OUTOFMEMORY) {
				critical_error("Out of memory", "");
				goto cleanup_l;
			}
			if (error == ERR_CRITICAL) {
				goto cleanup_l;
			}
			if (error) {
				break;
			}
		}
	}
	
	if ((g_workspace == NULL) || !strcmp(g_workspace, "")) {
		log_critical_error(__func__, "\"workspace\" has not been set.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	string_copy_c(&tempString, g_workspace);
	error = vfs_init(&g_vfs, &tempString);
	if (error) {
		log_critical_error(__func__, "Could not start VFS");
		goto cleanup_l;
	}
	



	/*  As of SDL v2.0.14:
		Allocates 220 bytes that SDL_Quit doesn't free.
		I don't think I can do anything about it. */
	error = SDL_Init(SDL_INIT_TIMER);
	if (error != 0) {
		critical_error("SDL_Init returned %s", SDL_GetError());
		return ERR_CRITICAL;
	}
	
	entity_initEntityList();
	modelList_init();
	
	error = snetwork_init();
	if (error) {
		critical_error("Could not initialize network", "");
		return ERR_CRITICAL;
	}
	
	error = terminal_terminalInit();
	if (error) {
		critical_error("Could not initialize the terminal", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	string_free(&tempString);

	return error;
}

static void main_quit(void) {
	
	terminal_quitConsole();
	
	modelList_free();
	entity_freeEntityList();
	
	snetwork_quit();
	
	SDL_Quit();
	
}

int main(int argc, char *argv[]) {
	
	int error = 0;
	lua_State *Lua;
	const char *luaFileName = "smain.lua";
	string_t luaFilePath;
	cfg2_var_t *lua_main_v;
	string_t tempString;
	
	log_info(__func__, "Starting engine-1 v0.0 (Server)");
	
	string_init(&tempString);
	string_init(&luaFilePath);
	
	error = main_init(argc, argv, Lua);
	if (error) {
		error = ERR_CRITICAL;
		goto cleanup_l;
	}

	lua_main_v = cfg2_findVar(CFG_LUA_MAIN);
	if (lua_main_v == NULL) {
		log_critical_error(__func__, "\""CFG_LUA_MAIN"\" does not exist.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	if (!strcmp(lua_main_v->string, "")) {
		log_critical_error(__func__, "\""CFG_LUA_MAIN"\" has not been set.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}

	/* @TODO: Do proper file path sanitization. */
	string_copy_c(&luaFilePath, g_workspace);
	file_concatenatePath(&luaFilePath, string_const(lua_main_v->string));
	file_concatenatePath(&luaFilePath, string_const(luaFileName));
	
	if (g_cfg2.quit) {
		error = ERR_OK;
		goto cleanup_l;
	}
	
	/* Before we begin, lock the restricted variables. */
	g_cfg2.adminLevel = cfg2_admin_game;
	
	log_info(__func__, "Executing \"%s\"", luaFilePath.value);
	error = lua_sandbox_init(&Lua, luaCFunctions, luaFilePath.value);
	lua_sandbox_quit(&Lua);
	if (error) {
		log_critical_error(__func__, "Could not start Lua server");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	main_quit();
	
	vfs_free(&g_vfs);
	
	terminal_quitConsole();
	cfg2_free();
	
	string_free(&luaFilePath);
	string_free(&tempString);
	
	// I'm leaving this because it works and it's cool.
	info("Server quit (%s)", (char*[4]){"OK", "Error", "Critical error", "Out of memory"}[error]);

    return error;
}
