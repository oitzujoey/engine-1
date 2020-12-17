
#include <config.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "render.h"
#include "input.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

extern SDL_Window* window;
extern SDL_Surface* screenSurface;

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

int
main (int argc, char* argv[])
{

	int error = 0;
	SDL_Event event;
	
	puts("Starting " PACKAGE_NAME " v" PACKAGE_VERSION " (Client)");
	
	if (error = windowInit()) {
		fprintf(stderr, "Error: windowInit returned %i | Cannot continue\n", error);
		return 0;
	}
	
	render();
	
	while(!getInput(&event)) {
		SDL_Delay(10);
	}
	
	windowQuit();

	lua_State* Lua;
	Lua = luaL_newstate();
	luaL_openlibs(Lua);
	luaL_loadfile(Lua, "src/server/main.lua");
	lua_pcall(
		Lua,
		0,
		0,
		0
	);
	lua_close(Lua);
	/* lua_dofile(Lua, "src/server/main.lua") */
	
	puts("Shutdown successfully");
	
	return 0;
}
