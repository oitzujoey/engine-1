
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <SDL2/SDL.h>
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
#include "../common/terminal.h"

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

extern SDL_Window *g_window;
// extern SDL_Surface *g_screenSurface;

luaCFunc_t luaCFunctions[] = {
	{.func = l_log_info,            .name = "l_log_info"},
	{.func = render,                .name = "render"},
	{.func = getInput,              .name = "getInput"},
	{.func = l_obj_loadOoliteDAT,   .name = "l_loadOoliteModel"},
	// {.func = l_cnetwork_receive,    .name = "l_snetwork_receive"},
	{.func = NULL,                  .name = NULL}
};

// const cfg_var_init_t initialConfigVars[] = {
// 	{.name = "client",                  .vector = 0,    .integer = 0,                               .string = NULL,                         .type = none,       .handle = NULL,                                     .permissions = CFG_VAR_FLAG_NONE},
// 	{.name = "lua_main",                .vector = 0,    .integer = 0,                               .string = "",                           .type = string,     .handle = NULL,                                     .permissions = CFG_VAR_FLAG_READ},
// 	{.name = "workspace",               .vector = 0,    .integer = 0,                               .string = "",                           .type = string,     .handle = vfs_handle_setWorkspace,                  .permissions = CFG_VAR_FLAG_READ},
// 	{.name = CFG_PORT,                  .vector = 0,    .integer = CFG_PORT_DEFAULT,                .string = "",                           .type = integer,    .handle = cnetwork_handle_setServerPort,            .permissions = CFG_VAR_FLAG_READ},
// 	{.name = CFG_CONNECTION_TIMEOUT,    .vector = 0,    .integer = CFG_CONNECTION_TIMEOUT_DEFAULT,  .string = "",                           .type = integer,    .handle = network_handle_connectionTimeout,         .permissions = CFG_VAR_FLAG_READ},
// 	{.name = CFG_IP_ADDRESS,            .vector = 0,    .integer = 0,                               .string = "localhost",                  .type = string,     .handle = cnetwork_handle_setIpAddress,             .permissions = CFG_VAR_FLAG_READ},
// 	{.name = CFG_MAX_RECURSION,         .vector = 0,    .integer = CFG_MAX_RECURSION_DEFAULT,       .string = "",                           .type = integer,    .handle = cfg_handle_maxRecursion,                  .permissions = CFG_VAR_FLAG_READ},
// 	{.name = CFG_RUN_QUIET,             .vector = 0,    .integer = false,                           .string = "",                           .type = integer,    .handle = NULL,                                     .permissions = CFG_VAR_FLAG_READ | CFG_VAR_FLAG_WRITE},
// 	{.name = CFG_HISTORY_LENGTH,        .vector = 0,    .integer = CFG_HISTORY_LENGTH_DEFAULT,      .string = "",                           .type = integer,    .handle = cfg_handle_updateCommandHistoryLength,    .permissions = CFG_VAR_FLAG_READ | CFG_VAR_FLAG_WRITE},
// 	{.name = CFG_OPENGL_LOG_FILE,       .vector = 0,    .integer = 0,                               .string = CFG_OPENGL_LOG_FILE_DEFAULT,  .type = string,     .handle = render_handle_updateLogFileName,          .permissions = CFG_VAR_FLAG_READ},
// 	{.name = NULL,                      .vector = 0,    .integer = 0,                               .string = NULL,                         .type = none,       .handle = NULL,                                     .permissions = CFG_VAR_FLAG_NONE}
// };

const cfg2_var_init_t g_clientVarInit[] = {
	// Commands
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

static void main_housekeeping(void) {
	int error = ERR_CRITICAL;
	
	SDL_Event event;
	
	// This must come first to allow network disconnect.
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            g_cfg2.quit = true;
		}
    }

	error = terminal_runTerminalCommand();
	if (error) {
		goto cleanup_l;
	}

	error = cnetwork_runEvents();
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

	// SDL_Delay(10);
	
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

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
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
	
	error = render_initOpenGL();
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

static void windowQuit(void) {

	SDL_DestroyWindow(g_window);
	g_window = NULL;
	
	SDL_Quit();
}

static int main_init(const int argc, char *argv[], lua_State *luaState) {
	int error;
	
	string_t tempString;
	
	error = string_init(&tempString);
	if (error) {
		critical_error("Out of memory", "");
		goto cleanup_l;
	}
	
	info("Initializing client vars", "");
	
	cfg2_init(luaState);
	
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
	
	error = cfg2_createVariables(g_clientVarInit);
	if (error) {
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
		g_cfg2.recursionDepth = 0;
		cfg2_execFile(AUTOEXEC);
	}

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			string_copy_c(&tempString, argv[i]);
			g_cfg2.recursionDepth = 0;
			error = cfg2_execString(&tempString, "Console");
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
	
	error = windowInit();
	if (error) {
		critical_error("windowInit returned %i", error);
		error = ERR_CRITICAL;
		goto cleanup_l;
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
	
	entity_initEntityList();
	modelList_init();
	
	error = cnetwork_init();
	if (error) {
		critical_error("cnetwork_init failed with error %i", error);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
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
	
	error = 0;
	cleanup_l:
	
	string_free(&tempString);
	
	return error;
}

void main_quit(void) {

	terminal_quitConsole();

	cnetwork_quit();
	
	modelList_free();
	entity_freeEntityList();
	
	vfs_free(&g_vfs);
	
	windowQuit();
	
	cfg2_free();
}

int main (int argc, char *argv[]) {

	int error = 0;
	lua_State *Lua;
	const char *luaFileName = "cmain.lua";
	string_t luaFilePath;
	cfg2_var_t *v_luaMain;
	
	info("Starting engine-1 v0.0 (Client)", "");
	
	error = string_init(&luaFilePath);
	if (error) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	error = main_init(argc, argv, Lua);
	if (error) {
		critical_error("main_init returned %i", error);
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
	string_copy_c(&luaFilePath, g_workspace);
	file_concatenatePath(&luaFilePath, string_const(v_luaMain->string));
	file_concatenatePath(&luaFilePath, string_const(luaFileName));
	
	if (g_cfg2.quit) {
		error = ERR_OK;
		goto cleanup_l;
	}
	
	/* Before we begin, lock the restricted variables. */
	g_cfg2.adminLevel = cfg2_admin_game;
	
	// Start Lua.
	
	log_info(__func__, "Executing \"%s\"", luaFilePath.value);
	error = lua_sandbox_init(&Lua, luaCFunctions, luaFilePath.value);
	if (error) {
		error("Could not start Lua client.", "");
		error = ERR_CRITICAL;
		goto luaCleanup_l;
	}
	
    // Run startup.
    
	error = lua_runFunction(Lua, "startup", MAIN_LUA_STARTUP_TIMEOUT);
    if (error) {
        error = ERR_CRITICAL;
        goto luaCleanup_l;
    }
	
	// Run the main game.
	
	while (!g_cfg2.quit) {
	
		// Set timeout
	
		error = lua_runFunction(Lua, "main", MAIN_LUA_MAIN_TIMEOUT);
		if (error) {
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		
        main_housekeeping();
	}
	
	// Run shutdown.
	
	error = lua_runFunction(Lua, "shutdown", MAIN_LUA_SHUTDOWN_TIMEOUT);
    if (error) {
        error = ERR_CRITICAL;
        goto luaCleanup_l;
    }
	
	// Cleanup.
	
	error = 0;
	luaCleanup_l:
	
	lua_sandbox_quit(&Lua);
	
	cleanup_l:

	main_quit();
	
	string_free(&luaFilePath);
	
	info("Client quit (%i)", error);
	
	return 0;
}
