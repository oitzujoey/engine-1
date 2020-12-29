
#include "lua_common.h"
#include <stdlib.h>
#include "log.h"

int l_common_toString(lua_State *luaState) {
	
	int argc = lua_gettop(luaState);
	if (argc != 1) {
		error("Requires 1 argument", "");
		lua_error(luaState);
	}
	
	if (lua_isstring(luaState, 1)) {
		warning("Are you sure you wanted to pass a string?", "");
		lua_pushstring(luaState, lua_tostring(luaState, 1));
	}
	else if (lua_isinteger(luaState, 1)) {
		lua_pushfstring(luaState, "%i", lua_tointeger(luaState, 1));
	}
	
	return 1;
}
