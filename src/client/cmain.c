
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <SDL2/SDL.h>
#include <physfs.h>
#include <string.h>
#include "render.h"
#include "input.h"
#include "../common/common.h"
#include "../common/lua_sandbox.h"
#include "cnetwork.h"
#include "../common/log.h"
#include "../common/cfg2.h"
#include "../common/file.h"
#include "../common/network.h"
#include "../common/entity.h"
#include "../common/obj.h"
#include "../common/vfs.h"
#ifndef NOTERMINAL
#include "../common/terminal.h"
#endif
#include "../common/lua_common.h"
#include "../common/str2.h"
#include "material.h"
#include "../common/memory.h"
#include "../common/random.h"

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

extern SDL_Window *g_window;
// extern SDL_Surface *g_screenSurface;

extern material_list_t g_materialList;

luaCFunc_t luaClientFunctions[] = {
	// {.func = render,                .name = "render"},
	// {.func = getInput,              .name = "getInput"},
	// {.func = l_cnetwork_receive,    .name = "l_snetwork_receive"},
	{.func = l_material_create,             .name = "material_create"},
	{.func = l_model_linkDefaultMaterial,   .name = "model_linkDefaultMaterial"},
	{.func = l_entity_linkMaterial,         .name = "entity_linkMaterial"},
	{.func = NULL,                          .name = NULL}
};

const cfg2_var_init_t g_clientVarInit[] = {
	// Commands
	{
		.name = "bind",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_bind
	},
	{
		.name = "bindMouse",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_bindMouse
	},
	// Variables
	{
		.name = "client",
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
		.callback = cnetwork_callback_setServerPort
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
		.callback = cnetwork_callback_setIpAddress
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
	
    error = render(&g_entityList.entities[0]);
    // if (error) {
    //     goto cleanup_l;
    // }
    
    error = input_processSdlEvents(luaState);
	if (error) {
		goto cleanup_l;
	}

#ifndef NOTERMINAL
	error = terminal_runTerminalCommand(luaState);
	if (error) {
		goto cleanup_l;
	}
#endif

	error = cnetwork_runEvents(luaState);
	if (error) {
		goto cleanup_l;
	}

	// for (int i = 0; i < g_entityList.entities_length; i++) {
	//     for (int j = 0; j < g_entityList.deletedEntities_length; j++) {
	//         if (g_entityList.deletedEntities[j] == i) {
	//             continue;
	//         }
	//     }
	//     entity_printEntity(i);
	// }

	// SDL_Delay(8);
	
	error = ERR_OK;
	cleanup_l:
	if (error) {
		g_cfg2.quit = true;
	}
}

int windowInit(void) {
	int error = ERR_CRITICAL;

	int sdlFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;

	g_window = NULL;
	// g_screenSurface = NULL;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
		fprintf(stderr, "Error: SDL could not initialize | SDL_Error %s\n", SDL_GetError());
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = SDL_GetDesktopDisplayMode(0, &g_displayMode);
	if (error < 0) {
		error("Could not get desktop display mode. SDL error: %s", SDL_GetError());
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	g_window = SDL_CreateWindow("engine-1 Client", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, g_displayMode.w, g_displayMode.h, sdlFlags);
	if (g_window == NULL) {
		fprintf(stderr, "Error: Window could not be created | SDL_Error %s\n", SDL_GetError());
		error = ERR_GENERIC;
		goto cleanup_l;
	}

	error = SDL_SetRelativeMouseMode(SDL_TRUE);
	if (error < 0) {
		error("Relative mouse mode is not supported on this platform. It will be hard to use the mouse", "");
	}
	
	error = render_initOpenGL();
	if (error) {
		goto cleanup_l;
	}
	
	error = material_initList(&g_materialList);
	if (error) {
		critical_error("Failed to initialize material list.", "");
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

static void windowQuit(void) {

	material_freeList(&g_materialList);
	
	SDL_DestroyWindow(g_window);
	g_window = NULL;
	
	SDL_Quit();
}

static int main_init(const int argc, char *argv[], lua_State *luaState) {
	int error;
	
	char *tempString = NULL;

	(void) random_init();

	error = vfs_init((uint8_t *) argv[0]);
	if (error) goto cleanup_l;

	error = input_init();
	if (error) {
		critical_error("Could not initialize user input", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	info("Initializing client vars", "");
	
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
	
	error = cfg2_createVariables(g_clientVarInit, luaState);
	if (error) {
		log_critical_error(__func__, "Could not load initial config vars due to bad initialization table.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	else if (error == ERR_OUTOFMEMORY) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}

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
	
	error = windowInit();
	if (error) {
		critical_error("windowInit returned %i", error);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}

	if (g_workspace.str_length == 0) {
		log_critical_error(__func__, "\"workspace\" has not been set.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// string_copy_c(&tempString, g_workspace);
	// error = vfs_init(&g_vfs, &tempString);
	// if (error) {
	// 	log_critical_error(__func__, "Could not start VFS");
	// 	goto cleanup_l;
	// }
	
	entity_initEntityList();
	modelList_init();
	
	error = cnetwork_init();
	if (error) {
		critical_error("cnetwork_init failed with error %i", error);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
#ifndef NOTERMINAL
	error = terminal_initConsole();
	if (error) {
		goto cleanup_l;
	}
	error = terminal_terminalInit();
	if (error) {
		critical_error("Could not initialize the terminal", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
#endif
	
	error = 0;
	cleanup_l:
	
	MEMORY_FREE(&tempString);
	
	return error;
}

void main_quit(void) {

#ifndef NOTERMINAL
	terminal_quitConsole();
	terminal_terminalQuit();
#endif
	
	cnetwork_quit();
	
	modelList_free();
	entity_freeEntityList();
	
	// vfs_free(&g_vfs);
	
	windowQuit();
	
	cfg2_free();
}

static uint32_t main_callback_block(uint32_t interval, void *param) {
	*((bool *) param) = true;
	return 0;
}

int main (int argc, char *argv[]) {

	int error = 0;
	lua_State *luaState = NULL;
	const char *luaFileName = "cmain.lua";
	char *luaFilePath = NULL;
	cfg2_var_t *v_luaMain;
	bool proceed;
	
	info("Starting engine-1 v0.0 (Client)", "");
	
	error = main_init(argc, argv, luaState);
	if (error) {
		critical_error("main_init returned an error.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	v_luaMain = cfg2_findVar(CFG_LUA_MAIN);
	if (v_luaMain == NULL) {
		log_critical_error(__func__, "\""CFG_LUA_MAIN"\" does not exist.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	if (!strcmp(v_luaMain->string, "")) {
		log_critical_error(__func__, "\""CFG_LUA_MAIN"\" has not been set.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}

	/* @TODO: Do proper file path sanitization. */
	str2_copyMalloc(&luaFilePath, v_luaMain->string);
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
		error("Could not initialize Lua client.", "");
		error = ERR_CRITICAL;
		goto luaCleanup_l;
	}
	
	lua_sandbox_addFunctions(&luaState, luaCommonFunctions);
	lua_sandbox_addFunctions(&luaState, luaClientFunctions);
	
	// Create table `NETWORK_LUA_CLIENTSTATE_NAME`.
	lua_newtable(luaState);
	lua_setglobal(luaState, NETWORK_LUA_CLIENTSTATE_NAME);
	
	lua_newtable(luaState);
	lua_setglobal(luaState, NETWORK_LUA_SERVERSTATE_NAME);
    // Run startup.
    
	error = lua_runFunction(luaState, "startup", MAIN_LUA_STARTUP_TIMEOUT);
    if (error) {
        error = ERR_CRITICAL;
        goto luaCleanup_l;
    }
	
	// Run the main game.
	
	while (!g_cfg2.quit) {
        main_housekeeping(luaState);

        error = lua_runFunction(luaState, "main", MAIN_LUA_MAIN_TIMEOUT);
		if (error) {
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
	}
	
	// Run shutdown.
	
	error = lua_runFunction(luaState, "shutdown", MAIN_LUA_SHUTDOWN_TIMEOUT);
    if (error) {
        error = ERR_CRITICAL;
        goto luaCleanup_l;
    }
	
	// Cleanup.
	
	error = 0;
	luaCleanup_l:
	
	lua_sandbox_quit(&luaState);
	
	cleanup_l:

	main_quit();
	
	MEMORY_FREE(&luaFilePath);
	
	info("Client quit (%i)", error);
	
	return 0;
}
