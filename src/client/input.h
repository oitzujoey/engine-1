
#ifndef INPUT_H
#define INPUT_H

#include "../common/types.h"

int input_init(void);
int input_processSdlEvents(lua_State *luaState);
int input_bind(const char * const key, const char * const downCommand, const char * const upCommand);

#endif
