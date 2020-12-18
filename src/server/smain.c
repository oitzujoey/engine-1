
#include <stdio.h>
#include "../common/common.h"
#include "../common/lua_client.h"
#include "../common/obj.h"

luaCFunc_t luaCFunctions[] = {
    {.func = l_puts,    .name = "l_puts"    },
    {.func = l_loadObj, .name = "l_loadObj" },
	{.func = NULL,      .name = NULL        }
};

int main(int argc, char *argv[]) {
	
	int error = 0;
	lua_State *Lua;
	
	puts("Starting engine-1 v0.0 (Server)");
		
	if (argc != 2) {
		fprintf(stderr, "Error: engine-1 must have one argument\n");
		return 0;
	}
	
	luaInit(&Lua, luaCFunctions, argv[1]);
	
	luaQuit(&Lua);
	
	puts("Server quit");

    return 0;
}
