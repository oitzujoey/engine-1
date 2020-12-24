
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

int windowInit() {

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

int windowQuit() {

	SDL_FreeSurface(screenSurface);
	screenSurface = NULL;

	SDL_DestroyWindow(window);
	window = NULL;
	
	SDL_Quit();
}

int main (int argc, char *argv[]) {

	int error = 0;
	lua_State *Lua;
	string_t data;
	
	puts("Starting engine-1 v0.0 (Client)");
	
	string_init(&data);
	
	if (error = windowInit()) {
		fprintf(stderr, "Error: windowInit returned %i | Cannot continue\n", error);
		return 0;
	}
	
	cnetwork_init("192.168.1.136");
	
	do {
		error = l_cnetwork_receive((Uint8 *) data.value, (int *) &data.length);
	} while (!error && (data.length == 0));
	
	if (error) {
		error("l_cnetwork_receive returned %i", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	string_print(&data);
	
	// if (argc != 2) {
	// 	fprintf(stderr, "Error: engine-1 must have one argument\n");
	// 	return 0;
	// }

	// lua_sandbox_init(&Lua, luaCFunctions, argv[1]);
	
	// lua_sandbox_quit(&Lua);

	cleanup_l:

	cnetwork_quit();

	windowQuit();
	
	string_free(&data);

	puts("Client quit");
	
	return 0;
}
