
#ifndef INPUT_H
#define INPUT_H

#include "../common/types.h"

int input_init(void);
int input_processSdlEvents(lua_State *luaState);
int input_bind(const uint8_t * const key, const uint8_t * const downCommand, const uint8_t * const upCommand);
int input_bindMouse(const uint8_t *callback);

#endif
