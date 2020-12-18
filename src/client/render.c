
#include "render.h"

SDL_Window* window;
SDL_Surface* screenSurface;

int render(lua_State *L) {

	SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
	
	SDL_UpdateWindowSurface(window);
	
	return 0;
}
