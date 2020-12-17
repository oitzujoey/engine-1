
#include "input.h"

int getInput(SDL_Event* event) {
    
    if (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT) {
            return 1;
        }
    }
    return 0;
}

int getKeys() {

}

int getMouse() {

}
