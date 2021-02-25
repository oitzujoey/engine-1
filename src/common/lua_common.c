
#include "lua_common.h"
#include <stdlib.h>
#include "log.h"
#include "obj.h"
#include "vfs.h"
#include "entity.h"
#include "vector.h"

luaCFunc_t luaCommonFunctions[] = {
	{.func = l_common_puts,             .name = "puts"},
	// {.func = l_loadObj,                 .name = "loadObj"},
	{.func = l_log_info,                .name = "info"},
	{.func = l_log_warning,             .name = "warning"},
	{.func = l_log_error,               .name = "error"},
	{.func = l_log_critical_error,      .name = "critical_error"},
	// {.func = l_vfs_getFileText,         .name = "vfs_getFileText"},
	{.func = l_obj_loadOoliteDAT,       .name = "loadOoliteModel"},
	{.func = l_common_toString,         .name = "toString"},
	{.func = l_entity_createEntity,     .name = "createEntity"},
	{.func = l_entity_linkChild,        .name = "entity_linkChild"},
	{.func = l_entity_setPosition,      .name = "entity_setPosition"},
	{.func = l_entity_setOrientation,   .name = "entity_setOrientation"},
	{.func = l_hamiltonProduct,         .name = "hamiltonProduct"},
	{.func = l_quatNormalize,           .name = "quatNormalize"},
	// {.func = l_cnetwork_receive,    .name = "l_snetwork_receive"},
	{.func = NULL,                  .name = NULL}
};

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
