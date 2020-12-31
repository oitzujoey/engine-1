
#include "input.h"

int getInput(lua_State *luaState) {
    
	SDL_Event event;
    
    if (SDL_PollEvent(&event)) {
        lua_pushinteger(luaState, event.type);
        lua_pushinteger(luaState, event.key.keysym.mod);
        return 2;
    }
    
    return 0;
}

// int getKeys(void) {

// }

// int getMouse(void) {

// }
