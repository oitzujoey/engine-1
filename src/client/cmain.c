
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

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

extern SDL_Window *window;
extern SDL_Surface *screenSurface;

luaCFunc_t luaCFunctions[] = {
	{.func = l_puts,                .name = "puts"              },
	{.func = render,                .name = "render"            },
	{.func = getInput,              .name = "getInput"          },
	// {.func = l_cnetwork_receive,    .name = "l_snetwork_receive"},
	{.func = NULL,                  .name = NULL                }
};

const cfg_var_init_t initialConfigVars[] = {
	{.name = "client",          .vector = 0,    .integer = 0,                   .string = NULL,         .type = none,       .permissions = CFG_VAR_PERMISSION_NONE},
	{.name = "lua_main",        .vector = 0,    .integer = 0,                   .string = "",           .type = string,     .permissions = CFG_VAR_PERMISSION_READ},
	{.name = "workspace",       .vector = 0,    .integer = 0,                   .string = "",           .type = string,     .permissions = CFG_VAR_PERMISSION_READ},
	{.name = "server_port",     .vector = 0,    .integer = DEFAULT_PORT_NUMBER, .string = "",           .type = integer,    .permissions = CFG_VAR_PERMISSION_READ},
	{.name = "client_port",     .vector = 0,    .integer = DEFAULT_PORT_NUMBER, .string = "",           .type = integer,    .permissions = CFG_VAR_PERMISSION_READ},
	{.name = "ip_address",      .vector = 0,    .integer = 0,                   .string = "localhost",  .type = string,     .permissions = CFG_VAR_PERMISSION_READ},
	{.name = NULL,              .vector = 0,    .integer = 0,                   .string = NULL,         .type = none,       .permissions = CFG_VAR_PERMISSION_NONE}
};

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
	
	cfg_var_t *workspace_v;
	string_t execString;
	
	error = string_init(&execString);
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
		cfg_execFile(AUTOEXEC);
	}

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			string_copy_c(&execString, argv[i]);
			cfg_execString(&execString, "Console");
		}
	}
	
	error = windowInit();
	if (error) {
		critical_error("windowInit returned %i", error);
		error = ERR_CRITICAL;
		goto cleanup_l;
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
	
	error = vfs_init(&g_vfs, &workspace_v->string);
	if (error) {
		log_critical_error(__func__, "Could not start VFS");
		goto cleanup_l;
	}
	
	error = cnetwork_init();
	if (error) {
		critical_error("cnetwork_init failed with error %i", error);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = 0;
	cleanup_l:
	
	string_free(&execString);
	
	return error;
}

void main_quit(void) {

	cnetwork_quit();
	
	vfs_free(&g_vfs);
	
	windowQuit();
	
	cfg_free();
}

int main (int argc, char *argv[]) {

	int error = 0;
	string_t data;
	
	puts("Starting engine-1 v0.0 (Client)");
	
	string_init(&data);
	
	error = main_init(argc, argv);
	if (error) {
		critical_error("main_init returned %i", error);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	do {
		error = l_cnetwork_receive((Uint8 **) &data.value, (int *) &data.length);
	} while (!error && (data.length == 0));
	
	if (error) {
		error("l_cnetwork_receive returned %i", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	--data.length;
	string_print(&data);
	
	// if (argc != 2) {
	// 	fprintf(stderr, "Error: engine-1 must have one argument\n");
	// 	return 0;
	// }

	// lua_sandbox_init(&Lua, luaCFunctions, argv[1]);
	
	// lua_sandbox_quit(&Lua);

	error = 0;
	cleanup_l:

	main_quit();
	
	string_free(&data);

	info("Client quit (%i)", error);
	
	return 0;
}