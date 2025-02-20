#include "lua_common.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "log.h"
#include "obj.h"
#include "vfs.h"
#include "entity.h"
#include "vector.h"
#include "cfg2.h"
#include "lua_sandbox.h"
#include "str2.h"
#include "parse_mesh.h"
#include "lua_sqlite.h"

luaCFunc_t luaCommonFunctions[] = {
	{.func = l_common_puts,             .name = "puts"},
	{.func = l_common_toString,         .name = "toString"},
	{.func = l_common_sin,              .name = "sin"},
	{.func = l_common_cos,              .name = "cos"},
	{.func = l_common_round,            .name = "round"},
	{.func = l_common_abs,              .name = "abs"},
	{.func = l_common_sqrt,             .name = "sqrt"},
	{.func = l_common_random,           .name = "random"},
	{.func = l_log_info,                .name = "info"},
	{.func = l_log_warning,             .name = "warning"},
	{.func = l_log_error,               .name = "error"},
	{.func = l_log_critical_error,      .name = "critical_error"},
	{.func = l_obj_loadOoliteDAT,       .name = "loadOoliteModel"},
	{.func = l_cmsh_load,               .name = "cmsh_load"},
	{.func = l_rmsh_load,               .name = "rmsh_load"},
	{.func = l_mesh_load,               .name = "mesh_load"},
	{.func = l_entity_createEntity,     .name = "entity_createEntity"},
	{.func = l_entity_deleteEntity,     .name = "entity_deleteEntity"},
	{.func = l_entity_linkChild,        .name = "entity_linkChild"},
	{.func = l_entity_unlinkChild,      .name = "entity_unlinkChild"},
	{.func = l_entity_setPosition,      .name = "entity_setPosition"},
	{.func = l_entity_setOrientation,   .name = "entity_setOrientation"},
	{.func = l_entity_setScale,         .name = "entity_setScale"},
	{.func = l_vec3_crossProduct,       .name = "vec3_crossProduct"},
	{.func = l_vec3_rotate,             .name = "vec3_rotate"},
	{.func = l_hamiltonProduct,         .name = "hamiltonProduct"},
	{.func = l_quatNormalize,           .name = "quatNormalize"},
	{.func = l_aaNormalize,             .name = "aaNormalize"},
	{.func = l_cfg2_setVariable,        .name = "cfg2_setVariable"},
	{.func = l_cfg2_setCallback,        .name = "cfg2_setCallback"},
	{.func = l_cfg2_getVariable,        .name = "cfg2_getVariable"},
	{.func = l_sqlite_open,             .name = "sqlite_open"},
	{.func = l_sqlite_exec,             .name = "sqlite_exec"},
	{.func = l_sqlite_close,            .name = "sqlite_close"},
	{.func = NULL,                  .name = NULL}
};


int l_common_random(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 0) {
		error("`random` does not accept any arguments", "");
		lua_error(l);
	}

	(void) lua_pushinteger(l, random());

	return 1;
}

int l_common_sqrt(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 1) {
		error("`sqrt` requires 1 argument", "");
		lua_error(l);
	}

	if (lua_isinteger(l, 1)) {
		lua_Integer s = lua_tointeger(l, 1);
		lua_Number n;
#ifdef DOUBLE_VEC
		n = sqrt(s);
#else
		n = sqrtf(s);
#endif
		(void) lua_pushnumber(l, n);
	}
	else if (lua_isnumber(l, 1)) {
		vec_t s = lua_tonumber(l, 1);
#ifdef DOUBLE_VEC
		s = sqrt(s);
#else
		s = sqrtf(s);
#endif
		(void) lua_pushnumber(l, s);
	}
	else {
		error("Argument of `sqrt` must be a number.", "");
		lua_error(l);
	}

	return 1;
}

int l_common_abs(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 1) {
		error("`abs` requires 1 argument", "");
		lua_error(l);
	}

	if (lua_isinteger(l, 1)) {
		lua_Integer s = lua_tointeger(l, 1);
		s = llabs(s);
		(void) lua_pushinteger(l, s);
	}
	else if (lua_isnumber(l, 1)) {
		vec_t s = lua_tonumber(l, 1);
#ifdef DOUBLE_VEC
		s = fabs(s);
#else
		s = fabsf(s);
#endif
		(void) lua_pushnumber(l, s);
	}
	else {
		error("Argument of `abs` must be a number.", "");
		lua_error(l);
	}

	return 1;
}

int l_common_round(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 1) {
		error("`round` requires 1 argument", "");
		lua_error(l);
	}

	vec_t s;

	if (lua_isinteger(l, 1)) {
		s = lua_tointeger(l, 1);
	}
	else if (lua_isnumber(l, 1)) {
		s = lua_tonumber(l, 1);
	}
	else {
		error("Argument of `round` must be a number.", "");
		lua_error(l);
	}

#ifdef DOUBLE_VEC
	s = round(s);
#else
	s = roundf(s);
#endif

	(void) lua_pushnumber(l, s);

	return 1;
}

int l_common_cos(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 1) {
		error("`cos` requires 1 argument", "");
		lua_error(l);
	}

	vec_t s;

	if (lua_isinteger(l, 1)) {
		s = lua_tointeger(l, 1);
	}
	else if (lua_isnumber(l, 1)) {
		s = lua_tonumber(l, 1);
	}
	else {
		error("Argument of `cos` must be a number.", "");
		lua_error(l);
	}

#ifdef DOUBLE_VEC
	s = cos(s);
#else
	s = cosf(s);
#endif

	(void) lua_pushnumber(l, s);

	return 1;
}

int l_common_sin(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 1) {
		error("`sin` requires 1 argument", "");
		lua_error(l);
	}

	vec_t s;

	if (lua_isinteger(l, 1)) {
		s = lua_tointeger(l, 1);
	}
	else if (lua_isnumber(l, 1)) {
		s = lua_tonumber(l, 1);
	}
	else {
		error("Argument of `sin` must be a number.", "");
		lua_error(l);
	}

#ifdef DOUBLE_VEC
	s = sin(s);
#else
	s = sinf(s);
#endif

	(void) lua_pushnumber(l, s);

	return 1;
}

int l_common_toString(lua_State *luaState) {
	
	int argc = lua_gettop(luaState);
	if (argc != 1) {
		error("Requires 1 argument", "");
		lua_error(luaState);
	}
	
	if (lua_isstring(luaState, 1)) {
		/* warning("Are you sure you wanted to pass a string?", ""); */
		lua_pushstring(luaState, lua_tostring(luaState, 1));
	}
	else if (lua_isnumber(luaState, 1)) {
		lua_pushfstring(luaState, "%d", lua_tonumber(luaState, 1));
	}
	else if (lua_isinteger(luaState, 1)) {
		lua_pushfstring(luaState, "%i", lua_tointeger(luaState, 1));
	}
	else if (lua_isboolean(luaState, 1)) {
		lua_pushfstring(luaState, "%s", lua_toboolean(luaState, 1) ? "true" : "false");
	}
	else if (lua_isnil(luaState, 1)) {
		lua_pushfstring(luaState, "nil");
	}
	else {
		lua_pushfstring(luaState, "<unhandled>");
	}
	
	return 1;
}

int l_cfg2_getVariable(lua_State *l) {
	int e = ERR_OK;

	int argc = lua_gettop(l);
	if (argc != 1) {
		error("Requires 1 argument", "");
		lua_error(l);
	}
	if (!lua_isstring(l, 1)) {
		error("Argument must be a string.", "");
		lua_error(l);
	}

	const char *variableName = lua_tostring(l, 1);
	cfg2_var_t *variable = cfg2_findVar(variableName);
	if (variable == NULL) {
		lua_pushnil(l);
		e = ERR_GENERIC;
		goto cleanup;
	}
	switch (variable->type) {
	case cfg2_var_type_none:
		lua_pushnil(l);
		break;
	case cfg2_var_type_vector:
		error("cfg2_var_type_vector: Not implemented. Variable name: %s", variableName);
		lua_pushnil(l);
		break;
	case cfg2_var_type_integer:
		lua_pushinteger(l, variable->integer);
		break;
	case cfg2_var_type_string:
		lua_pushstring(l, variable->string);
		break;
	}

 cleanup:
	(void) lua_pushinteger(l, e);
	return 2;
}

int l_cfg2_setVariable(lua_State *luaState) {
	int error = 0;
	
	int argc = lua_gettop(luaState);
	if (argc != 1) {
		error("Requires 1 argument", "");
		lua_error(luaState);
	}
	
	if (!lua_isstring(luaState, 1)) {
		error("Argument must be a string.", "");
		lua_error(luaState);
	}
	
	error = cfg2_execString(lua_tostring(luaState, 1), luaState, "Lua");
	if (error > ERR_GENERIC) {
		critical_error("Script returned a critical error. %i", error);
		lua_error(luaState);
	}
	lua_pushinteger(luaState, error);
	
	return 1;
}

/*
The callback that is used when Lua makes a callback.
*/
int cfg2_callback_callbackLua(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Simple!
	error = lua_runFunction(luaState, var->script, 10);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int l_cfg2_setCallback(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	cfg2_var_t *var;
	const char *varName = NULL;
	
	int argc = lua_gettop(luaState);
	if (argc != 2) {
		error("Requires 2 arguments", "");
		lua_error(luaState);
	}
	
	if (!lua_isstring(luaState, 1)) {
		error("Argument 1 must be a string.", "");
		lua_error(luaState);
	}
	
	if (!lua_isstring(luaState, 2)) {
		error("Argument 2 must be a string.", "");
		lua_error(luaState);
	}
	
	// Find variable.
	
	varName = lua_tostring(luaState, 1);
	
	var = cfg2_findVar(varName);
	if (var == NULL) {
		warning("Variable \"%s\" does not exist.", varName);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (g_cfg2.adminLevel < var->permissionCallback) {
		warning("Permissions not high enough to set callback for variable %s.", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Set callback.
	var->callback = cfg2_callback_callbackLua;
	
	// Set function name.
	str2_copyMalloc(&var->script, lua_tostring(luaState, 2));
	
	
	error = ERR_OK;
	cleanup_l:
	
	if (error > ERR_GENERIC) {
		critical_error("C call from Lua returned error: %i", error);
		lua_error(luaState);
	}
	
	lua_pushinteger(luaState, error);
	
	return 1;
}


int lua_common_printTable(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	if (lua_type(luaState, -2) == LUA_TSTRING) {
		printf("%s\n", lua_tostring(luaState, -2));
	}
	
	lua_pushnil(luaState);
	while (lua_next(luaState, -2)) {
		switch (lua_type(luaState, -1)) {
		case LUA_TSTRING:
			printf("%s : %s\n", lua_tostring(luaState, -2), lua_tostring(luaState, -1));
			break;
		case LUA_TNUMBER:
			if (lua_isinteger(luaState, -1)) {
				printf("%s : %lld\n", lua_tostring(luaState, -2), lua_tointeger(luaState, -1));
			}
			else {
				printf("%s : %f\n", lua_tostring(luaState, -2), lua_tonumber(luaState, -1));
			}
			break;
		case LUA_TBOOLEAN:
			printf("%s : %i\n", lua_tostring(luaState, -2), lua_toboolean(luaState, -1));
			break;
		case LUA_TTABLE:
			printf("{\n");
			error = lua_common_printTable(luaState);
			if (error) {
				goto cleanup_l;
			}
			printf("}\n");
			break;
		default:
			error("Unsupported type.", "");
			error = ERR_GENERIC;
			goto cleanup_l;
		}
		
		lua_pop(luaState, 1);
	}
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}
