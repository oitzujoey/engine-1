
#include <stdio.h>
#include "common.h"

int error;

vfs_t vfs_g;

int l_puts(lua_State *Lua) {
    printf(COLOR_CYAN"Lua {"COLOR_NORMAL"%s"COLOR_CYAN"}"COLOR_NORMAL"\n", lua_tostring(Lua, 1));
    return 0;
}

int com_sprintf(lua_State *Lua) {
    
}

int MSB(int n) {
	int i;
	
	for (i = 0; (1<<i) <= n; i++);
	
	return i - 1;
}
