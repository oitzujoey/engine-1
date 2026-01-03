
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "../SDL-release-2.32.8/include/SDL.h"
#include "../physfs-3.0.2/src/physfs.h"
#include "../lua-5.4.8/lua.h"
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

extern const uint8_t *g_pak;
extern const uint8_t *g_pak_end;

int l_main_checkQuit(lua_State *luaState);

luaCFunc_t luaServerFunctions[] = {
	{.func = l_snetwork_disconnect, .name = "network_disconnect"},
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
		.name = CFG_IP_ADDRESS,
		.vector = 0,
		.integer = 0,
		.string = CFG_IP_ADDRESS_DEFAULT,
		.type = cfg2_var_type_string,
		.permissionRead = cfg2_admin_game,
		.permissionWrite = cfg2_admin_supervisor,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = snetwork_callback_setIpAddress
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
	int e = ERR_CRITICAL;
	
	char *tempString = NULL;

	(void) random_init();
	
	// Start the VFS.

	e = vfs_init((uint8_t *) argv[0]);
	if (e) goto cleanup_l;
	
	// Start config system.
	
	log_info(__func__, "Initializing server vars");
	
	cfg2_init(luaState);
	
	e = cfg2_createVariables(g_commonVarInit, luaState);
	if (e == ERR_GENERIC) {
		log_critical_error(__func__, "Could not load initial config vars due to bad initialization table.");
		e = ERR_GENERIC;
		goto cleanup_l;
	}
	else if (e == ERR_OUTOFMEMORY) {
		outOfMemory();
		e = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	e = cfg2_createVariables(g_serverVarInit, luaState);
	if (e == ERR_GENERIC) {
		log_critical_error(__func__, "Could not load initial config vars due to bad initialization table.");
		e = ERR_GENERIC;
		goto cleanup_l;
	}
	else if (e == ERR_OUTOFMEMORY) {
		outOfMemory();
		e = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
#ifndef NOTERMINAL
	e = terminal_initConsole();
	if (e) {
		goto cleanup_l;
	}
#endif

	// Mount the zip file that was incbin'ed into this executable.
	e = PHYSFS_mountMemory((uint8_t *) &g_pak, (uint8_t *)&g_pak_end - (uint8_t *)&g_pak, NULL, "_______.zip", NULL, 1);
	if (!e) {
		error("Could not add embedded resources to the search path: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		e = ERR_GENERIC;
		goto cleanup_l;
	}

	// Execute autoexec.
	if (PHYSFS_exists(AUTOEXEC_CFG)) {
		info("Found \""AUTOEXEC_CFG"\"", "");
		g_cfg2.recursionDepth = 0;
		cfg2_execFile(AUTOEXEC_CFG, luaState);
	}
	else {
		warning("\""AUTOEXEC_CFG"\" not found.", "");
	}

	// Mount current directory.
	bool currentDirectoryMounted = false;
	e = PHYSFS_mount("./", "", true);
	if (!e) {
		warning("Could not add directory \"./\" to the search path: %s",
				PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		// This is not a major issue.
	}
	else {
		currentDirectoryMounted = true;
	}

	// Execute user settings config.
	if (PHYSFS_exists(SETTINGS_CFG)) {
		info("Found \""SETTINGS_CFG"\"", "");
		g_cfg2.recursionDepth = 0;
		cfg2_execFile(SETTINGS_CFG, luaState);
	}
	else {
		warning("\""SETTINGS_CFG"\" not found.", "");
	}

	// Unmount engine directory.
	if (currentDirectoryMounted) {
		e = PHYSFS_unmount("./");
		if (!e) {
			PHYSFS_ErrorCode errorCode = PHYSFS_getLastErrorCode();
			error("Could not remove directory \"./\" from the search path: %s",
				  PHYSFS_getErrorByCode(errorCode));
			e = ERR_GENERIC;
			goto cleanup_l;
		}
	}

	// Run command line arguments.

	cfg2_admin_t savedAdminLevel = g_cfg2.adminLevel;
	g_cfg2.adminLevel = cfg2_admin_supervisor;

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			str2_copyMalloc(&tempString, argv[i]);
			g_cfg2.recursionDepth = 0;
			e = cfg2_execString(tempString, luaState, "Console");
			if (e == ERR_OUTOFMEMORY) {
				outOfMemory();
				goto cleanup_l;
			}
			if (e == ERR_CRITICAL) {
				goto cleanup_l;
			}
			if (e) {
				break;
			}
		}
	}
	
	g_cfg2.adminLevel = savedAdminLevel;
	
	// Check for the workspace.
	
	if (g_workspace.str_length == 0) {
		log_critical_error(__func__, "\"workspace\" has not been set.");
		e = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Execute mod specific autoexec.
	// vfs_execAutoexec();



	/*  As of SDL v2.0.14:
		Allocates 220 bytes that SDL_Quit doesn't free.
		I don't think I can do anything about it. */
	e = SDL_Init(0);
	if (e != 0) {
		critical_error("SDL_Init returned %s", SDL_GetError());
		return ERR_CRITICAL;
	}
	
	entity_initEntityList();
	modelList_init();
	
	e = snetwork_init();
	if (e) {
		critical_error("Could not initialize network", "");
		return ERR_CRITICAL;
	}
	
#ifndef NOTERMINAL
	e = terminal_terminalInit();
	if (e) {
		critical_error("Could not initialize the terminal", "");
		e = ERR_CRITICAL;
		goto cleanup_l;
	}
#endif
	
	e = ERR_OK;
	cleanup_l:
	
	if (tempString) MEMORY_FREE(&tempString);

	return e;
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
	
	Uint64 performanceCounterValue = SDL_GetPerformanceCounter();
	Uint64 performanceCounterValue_last;
	while (!g_cfg2.quit) {
		proceed = false;

        main_housekeeping(luaState);

		performanceCounterValue_last = performanceCounterValue;
		performanceCounterValue = SDL_GetPerformanceCounter();
		vec_t deltaT = (vec_t) (performanceCounterValue - performanceCounterValue_last) / (vec_t) SDL_GetPerformanceFrequency();
		(void) lua_pushnumber(luaState, deltaT);
		(void) lua_setglobal(luaState, MAIN_LUA_DELTAT_NAME);

		error = lua_runFunction(luaState, "main", MAIN_LUA_MAIN_TIMEOUT);
		if (error) {
			error = ERR_CRITICAL;
			goto cleanup_l;
		}

		SDL_Delay(10);
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
	
	// Exit.
	
	// I'm leaving this because it works and it's cool.
	info("Server quit (%s)", (char*[4]){"OK", "Error", "Critical error", "Out of memory"}[error]);

    return error;
}
