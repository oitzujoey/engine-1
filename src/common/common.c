
#include <stdio.h>
#include "common.h"

int error;

vfs_t vfs;

int l_puts(lua_State *Lua) {
    printf(COLOR_CYAN"Lua "COLOR_NORMAL"%s\n", lua_tostring(Lua, 1));
    return 0;
}

int com_sprintf(lua_State *Lua) {
    
}
