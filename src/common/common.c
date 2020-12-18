
#include <stdio.h>
#include "common.h"

int com_puts(lua_State *Lua) {
    puts(lua_tostring(Lua, 1));
    return 0;
}

int com_sprintf(lua_State *Lua) {
    
}
