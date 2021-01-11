
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "render.h"
#include "input.h"
#include "../common/common.h"
#include "../common/lua_sandbox.h"
#include "cnetwork.h"
#include "../common/log.h"
#include "../common/cfg.h"
#include "../common/file.h"
#include "../common/network.h"
#include "../common/entity.h"
#include "../common/obj.h"
#include "../common/vfs.h"

static int l_main_checkQuit(lua_State *luaState);
static int l_main_housekeeping(lua_State *luaState);

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

extern SDL_Window *window;
extern SDL_Surface *screenSurface;

luaCFunc_t luaCFunctions[] = {
	{.func = l_log_info,            .name = "l_log_info"},
	{.func = render,                .name = "render"},
	{.func = getInput,              .name = "getInput"},
	{.func = l_main_housekeeping,   .name = "l_main_housekeeping"},
	{.func = l_main_checkQuit,      .name = "l_checkQuit"},
	{.func = l_obj_loadOoliteDAT,   .name = "l_loadOoliteModel"},
	{.func = l_main_housekeeping,   .name = "l_main_housekeeping"},
	// {.func = l_cnetwork_receive,    .name = "l_snetwork_receive"},
	{.func = NULL,                  .name = NULL}
};

const cfg_var_init_t initialConfigVars[] = {
	{.name = "client",                  .vector = 0,    .integer = 0,                               .string = NULL,         .type = none,       .handle = NULL,                                     .permissions = CFG_VAR_FLAG_NONE},
	{.name = "lua_main",                .vector = 0,    .integer = 0,                               .string = "",           .type = string,     .handle = NULL,                                     .permissions = CFG_VAR_FLAG_READ},
	{.name = "workspace",               .vector = 0,    .integer = 0,                               .string = "",           .type = string,     .handle = vfs_handle_setWorkspace,                  .permissions = CFG_VAR_FLAG_READ},
	{.name = CFG_PORT,                  .vector = 0,    .integer = CFG_PORT_DEFAULT,                .string = "",           .type = integer,    .handle = cnetwork_handle_setServerPort,            .permissions = CFG_VAR_FLAG_READ},
	{.name = CFG_CONNECTION_TIMEOUT,    .vector = 0,    .integer = CFG_CONNECTION_TIMEOUT_DEFAULT,  .string = "",           .type = integer,    .handle = network_handle_connectionTimeout,         .permissions = CFG_VAR_FLAG_READ},
	{.name = CFG_IP_ADDRESS,            .vector = 0,    .integer = 0,                               .string = "localhost",  .type = string,     .handle = cnetwork_handle_setIpAddress,             .permissions = CFG_VAR_FLAG_READ},
	{.name = CFG_MAX_RECURSION,         .vector = 0,    .integer = CFG_MAX_RECURSION_DEFAULT,       .string = "",           .type = integer,    .handle = cfg_handle_maxRecursion,                  .permissions = CFG_VAR_FLAG_READ},
	{.name = CFG_RUN_QUIET,             .vector = 0,    .integer = false,                           .string = "",           .type = integer,    .handle = NULL,                                     .permissions = CFG_VAR_FLAG_READ | CFG_VAR_FLAG_WRITE},
	{.name = CFG_HISTORY_LENGTH,        .vector = 0,    .integer = CFG_HISTORY_LENGTH_DEFAULT,      .string = "",           .type = integer,    .handle = cfg_handle_updateCommandHistoryLength,    .permissions = CFG_VAR_FLAG_READ | CFG_VAR_FLAG_WRITE},
	{.name = NULL,                      .vector = 0,    .integer = 0,                               .string = NULL,         .type = none,       .handle = NULL,                                     .permissions = CFG_VAR_FLAG_NONE}
};

static int l_main_checkQuit(lua_State *luaState) {
	lua_pushinteger(luaState, g_cfg.quit);
	return 1;
}

static int l_main_housekeeping(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	SDL_Event event;
	
	// This must come first to allow network disconnect.
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            g_cfg.quit = true;
		}
    }

	error = cfg_runTerminalCommand();
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

	SDL_Delay(10);
	
	error = ERR_OK;
	cleanup_l:
	if (error) {
		g_cfg.quit = true;
	}
	
	return 0;
}

int windowInit(void) {

	window = NULL;
	screenSurface = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Error: SDL could not initialize | SDL_Error %s\n", SDL_GetError());
		return 1;
	}
	else {
		window = SDL_CreateWindow("engine-1 Client", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			fprintf(stderr, "Error: Window could not be created | SDL_Error %s\n", SDL_GetError());
			return 2;
		}
		else {
			screenSurface = SDL_GetWindowSurface(window);
		}
	}
	return 0;
}

static void windowQuit(void) {

	SDL_FreeSurface(screenSurface);
	screenSurface = NULL;

	SDL_DestroyWindow(window);
	window = NULL;
	
	SDL_Quit();
}

static int main_init(const int argc, char *argv[]) {
	int error;
	
	string_t tempString;
	
	error = string_init(&tempString);
	if (error) {
		critical_error("Out of memory", "");
		goto cleanup_l;
	}
	
	info("Initializing server vars", "");
	
	error = cfg_initVars(initialConfigVars);
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
		cfg_execFile(AUTOEXEC, 0);
	}

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			string_copy_c(&tempString, argv[i]);
			error = cfg_execString(&tempString, "Console", 0);
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
	
	if (!strcmp(g_workspace, "")) {
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
	
	error = cfg_initConsole();
	if (error) {
		goto cleanup_l;
	}
	error = cfg_terminalInit();
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

	cfg_quitConsole();

	cnetwork_quit();
	
	modelList_free();
	entity_freeEntityList();
	
	vfs_free(&g_vfs);
	
	windowQuit();
	
	cfg_free();
}

int main (int argc, char *argv[]) {

	int error = 0;
	lua_State *Lua;
	const char *luaFileName = "cmain.lua";
	string_t luaFilePath;
	cfg_var_t *v_luaMain;
	
	info("Starting engine-1 v0.0 (Client)", "");
	
	error = string_init(&luaFilePath);
	if (error) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	error = main_init(argc, argv);
	if (error) {
		critical_error("main_init returned %i", error);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	v_luaMain = cfg_findVar("lua_main");
	if (v_luaMain == NULL) {
		log_critical_error(__func__, "\"lua_main\" does not exist.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	if (!strcmp(v_luaMain->string.value, "")) {
		log_critical_error(__func__, "\"lua_main\" has not been set.");
		error = ERR_GENERIC;
		goto cleanup_l;
	}

	/* @TODO: Do proper file path sanitization. */
	string_copy_c(&luaFilePath, g_workspace);
	file_concatenatePath(&luaFilePath, &v_luaMain->string);
	file_concatenatePath(&luaFilePath, string_const(luaFileName));
	
	if (g_cfg.quit) {
		error = ERR_OK;
		goto cleanup_l;
	}
	
	/* Before we begin, lock the restricted variables. */
	g_cfg.lock = true;
	
	log_info(__func__, "Executing \"%s\"", luaFilePath.value);
	error = lua_sandbox_init(&Lua, luaCFunctions, luaFilePath.value);
	lua_sandbox_quit(&Lua);

	if (error) {
		log_critical_error(__func__, "Could not start Lua server");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = 0;
	cleanup_l:

	main_quit();
	
	string_free(&luaFilePath);
	
	info("Client quit (%i)", error);
	
	return 0;
}
