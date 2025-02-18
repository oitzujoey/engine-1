
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <lua.h>
#include <physfs.h>
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
#ifndef NOTERMINAL
#include "../common/terminal.h"
#endif
#include "../common/str2.h"
#include "../common/memory.h"
#include "../common/random.h"

int l_main_checkQuit(lua_State *luaState);

luaCFunc_t luaServerFunctions[] = {
	{.func = NULL,                  .name = NULL}
};

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
		.type = cfg2_var_type_integer,
		.permissionRead = cfg2_admin_game,
		.permissionWrite = cfg2_admin_supervisor,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = snetwork_callback_setServerPort
	},
	{
		.name = CFG_MAX_CLIENTS,
		.vector = 0,
		.integer = CFG_MAX_CLIENTS_DEFAULT,
		.string = "",
		.type = cfg2_var_type_integer,
		.permissionRead = cfg2_admin_game,
		.permissionWrite = cfg2_admin_supervisor,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = snetwork_callback_maxClients
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

static void main_housekeeping(lua_State *luaState) {
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
	
	error = snetwork_runEvents(luaState);
	if (error) {
		goto cleanup_l;
	}
	
#ifndef NOTERMINAL
	/* Do terminal stuff */
	
	error = terminal_runTerminalCommand(luaState);
	if (error) {
		goto cleanup_l;
	}
#endif
	
	error = ERR_OK;
	cleanup_l:
	if (error) {
		g_cfg2.quit = true;
	}
	
	// SDL_Delay(8);
}

static int main_init(int argc, char *argv[], lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	char *tempString = NULL;

	(void) random_init();
	
	// Start the VFS.

	error = vfs_init((uint8_t *) argv[0]);
	if (error) goto cleanup_l;
	
	// Start config system.
	
	log_info(__func__, "Initializing server vars");
	
	cfg2_init(luaState);
	
	error = cfg2_createVariables(g_commonVarInit, luaState);
	if (error == ERR_GENERIC) {
		log_critical_error(__func__, "Could not load initial config vars due to bad initialization table.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	else if (error == ERR_OUTOFMEMORY) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	error = cfg2_createVariables(g_serverVarInit, luaState);
	if (error == ERR_GENERIC) {
		log_critical_error(__func__, "Could not load initial config vars due to bad initialization table.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	else if (error == ERR_OUTOFMEMORY) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
#ifndef NOTERMINAL
	error = terminal_initConsole();
	if (error) {
		goto cleanup_l;
	}
#endif

	// Execute main autoexec.

	// Mount engine directory.
	error = PHYSFS_mount("./", "", true);
	if (!error) {
		error("Could not add directory \"%s\" to the search path: %s", "./", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Execute autoexec.
	if (PHYSFS_exists(AUTOEXEC)) {
		info("Found \""AUTOEXEC"\"", "");
		g_cfg2.recursionDepth = 0;
		cfg2_execFile(AUTOEXEC, luaState);
	}
	
	// Unmount engine directory.
	error = PHYSFS_unmount("./");
	if (!error) {
		error("Could not remove directory \"%s\" from the search path: %s", "./", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		error = ERR_GENERIC;
		goto cleanup_l;
	}

	// Run command line arguments.

	cfg2_admin_t savedAdminLevel = g_cfg2.adminLevel;
	g_cfg2.adminLevel = cfg2_admin_supervisor;

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			str2_copyMalloc(&tempString, argv[i]);
			g_cfg2.recursionDepth = 0;
			error = cfg2_execString(tempString, luaState, "Console");
			if (error == ERR_OUTOFMEMORY) {
				outOfMemory();
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
	
	g_cfg2.adminLevel = savedAdminLevel;
	
	// Check for the workspace.
	
	if (g_workspace.str_length == 0) {
		log_critical_error(__func__, "\"workspace\" has not been set.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Execute mod specific autoexec.
	// vfs_execAutoexec();



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
	
#ifndef NOTERMINAL
	error = terminal_terminalInit();
	if (error) {
		critical_error("Could not initialize the terminal", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
#endif
	
	error = ERR_OK;
	cleanup_l:
	
	MEMORY_FREE(&tempString);

	return error;
}

static void main_quit(void) {
	
#ifndef NOTERMINAL
	terminal_quitConsole();
	terminal_terminalQuit();
#endif
	
	modelList_free();
	entity_freeEntityList();
	
	snetwork_quit();
	
	SDL_Quit();
	
}

static uint32_t main_callback_block(uint32_t interval, void *param) {
	*((bool *) param) = true;
	return 0;
}

int main(int argc, char *argv[]) {
	
	int error = 0;
	lua_State *luaState = NULL;
	const char *luaFileName = "smain.lua";
	char *luaFilePath = NULL;
	cfg2_var_t *lua_main_v;
	char *tempString = NULL;
	SDL_TimerID timerId;
	bool proceed;
	
	info("Starting engine-1 v0.0 (Server)", "");
	
	error = main_init(argc, argv, luaState);
	if (error) {
		critical_error("main_init returned an error.", "");
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
	str2_copyMalloc(&luaFilePath, lua_main_v->string);
	file_concatenatePath(&luaFilePath, luaFileName);
	
	if (g_cfg2.quit) {
		error = ERR_OK;
		goto cleanup_l;
	}
	
	/* Before we begin, lock the restricted variables. */
	g_cfg2.adminLevel = cfg2_admin_game;
	
	// Start Lua.
	
	error = lua_sandbox_init(&luaState, luaFilePath);
	if (error) {
		error("Could not initialize Lua server.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	lua_sandbox_addFunctions(&luaState, luaCommonFunctions);
	lua_sandbox_addFunctions(&luaState, luaServerFunctions);
	
    // Run startup.
    
	// Create number maxClients
	lua_pushinteger(luaState, MAX_CLIENTS);
	lua_setglobal(luaState, NETWORK_LUA_MAXCLIENTS_NAME);
	
	error = lua_runFunction(luaState, "startup", MAIN_LUA_STARTUP_TIMEOUT);
    if (error) {
        error = ERR_CRITICAL;
        goto luaCleanup_l;
    }
	
	// Create table `NETWORK_LUA_CLIENTSTATE_NAME`.
	lua_newtable(luaState);
	// for (int i = 0; i < MAX_CLIENTS; i++) {
	// 	// Indices start at one. :|
	// 	lua_pushinteger(luaState, i + 1);
	// 	lua_newtable(luaState);
	// 	/*
	// 	-1  0
	// 	-2  i
	// 	-3  clientState
	// 	*/
	// 	lua_settable(luaState, -3);
	// }
	lua_setglobal(luaState, NETWORK_LUA_CLIENTSTATE_NAME);
	
	lua_newtable(luaState);
	for (int i = 0; i < MAX_CLIENTS; i++) {
		// Indices start at one. :|
		lua_pushinteger(luaState, i + 1);
		lua_newtable(luaState);
		/*
		-1  0
		-2  i
		-3  serverState
		*/
		lua_settable(luaState, -3);
	}
	lua_setglobal(luaState, NETWORK_LUA_SERVERSTATE_NAME);
	
	// Run the main game.
	
	while (!g_cfg2.quit) {
	
		// Set timeout
	
		proceed = false;
		timerId = SDL_AddTimer(g_cfg2.maxFramerate, main_callback_block, &proceed);
	
        main_housekeeping(luaState);
        
		error = lua_runFunction(luaState, "main", MAIN_LUA_MAIN_TIMEOUT);
		if (error) {
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		
        while (!proceed) {
	        SDL_Delay(1);
        }
		SDL_RemoveTimer(timerId);
	}
	
	// Run shutdown.
	
	error = lua_runFunction(luaState, "shutdown", MAIN_LUA_SHUTDOWN_TIMEOUT);
    if (error) {
        error = ERR_CRITICAL;
        goto luaCleanup_l;
    }
	
	// Cleanup.
	
	error = ERR_OK;
	luaCleanup_l:
	
	lua_sandbox_quit(&luaState);
	
	cleanup_l:
	
	main_quit();
	
	// vfs_free(&g_vfs);
	
	cfg2_free();
	
	MEMORY_FREE(&luaFilePath);
	MEMORY_FREE(&tempString);
	
	// Exit.
	
	// I'm leaving this because it works and it's cool.
	info("Server quit (%s)", (char*[4]){"OK", "Error", "Critical error", "Out of memory"}[error]);

    return error;
}
