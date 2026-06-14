#include "lua_common.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../lua-5.4.8/lua.h"
#include "common.h"
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
	{.func = l_common_string_sub,       .name = "string_sub"},
	{.func = l_common_parse_double,     .name = "parse_double"},
	{.func = l_common_sin,              .name = "sin"},
	{.func = l_common_cos,              .name = "cos"},
	{.func = l_common_atan2,            .name = "atan2"},
	{.func = l_common_round,            .name = "round"},
	{.func = l_common_truncate,         .name = "truncate"},
	{.func = l_common_abs,              .name = "abs"},
	{.func = l_common_sqrt,             .name = "sqrt"},
	{.func = l_common_random,           .name = "random"},
	{.func = l_common_hash,             .name = "hash"},
	{.func = l_common_hash3d,           .name = "hash3d"},
	{.func = l_common_perlin,           .name = "perlin"},
	{.func = l_common_gridColor,        .name = "gridColor"},
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

uint64_t hash(uint64_t k) {
	k = (k ^ (k >> 30)) * 0xbf58476d1ce4e5b9;
	k = (k ^ (k >> 27)) * 0x94d049bb133111eb;
	k = k ^ (k > 31);
	return k % 4294967311;
}

uint64_t hash3d(uint32_t x, uint32_t y, uint32_t z) {
	return hash((((uint64_t) z) << 32) + (((uint64_t) y) << 16) + x);
}

uint32_t worleyDistance(uint32_t x0, uint32_t y0, uint32_t z0, uint32_t gridScale, uint32_t x1, uint32_t y1, uint32_t z1) {
	uint32_t ox = x0 / gridScale;
	uint32_t oy = y0 / gridScale;
	uint32_t oz = z0 / gridScale;
	uint64_t hashZYX = hash3d(ox + x1, oy + y1, oz + z1);
	uint32_t hashX = (hashZYX & 0xFFFF) % gridScale;
	uint32_t hashY = ((hashZYX >> 16) & 0xFFFF) % gridScale;
	uint32_t hashZ = ((hashZYX >> 32) & 0xFFFF) % gridScale;
	uint32_t dx = gridScale*(ox + x1) + hashX - x0;
	uint32_t dy = gridScale*(oy + y1) + hashY - y0;
	uint32_t dz = gridScale*(oz + z1) + hashZ - z0;
	uint32_t d2 = dx*dx + dy*dy + dz*dz;
	return d2;
}

uint32_t gridColor(uint32_t x, uint32_t y, uint32_t z) {
	const uint32_t gridScale = 30;
	{
		const uint64_t blendScale = 10;
		uint64_t offset = hash3d(x, y, z);
		/* offset = 0; */
		x += offset % blendScale;
		y += (offset >> 16) % blendScale;
		z += (offset >> 32) % blendScale;
	}
	uint8_t min_distance_x = 0;
	uint8_t min_distance_y = 0;
	uint8_t min_distance_z = 0;
	uint32_t min_distance = worleyDistance(x, y, z, gridScale, 0, 0, 0);
	for (uint8_t iz = -2; iz <= 2; iz++) {
		for (uint8_t iy = -2; iy <= 2; iy++) {
			for (uint8_t ix = -2; ix <= 2; ix++) {
				uint32_t d2 = worleyDistance(x, y, z, gridScale, ix, iy, iz);
				if (d2 < min_distance) {
					min_distance = d2;
					min_distance_x = ix;
					min_distance_y = iy;
					min_distance_z = iz;
				}
			}
		}
	}
	uint32_t color = hash3d(x/gridScale + min_distance_x, y/gridScale + min_distance_y, z/gridScale + min_distance_z);
	/* { */
	/* 	uint32_t brightness = 0; */
	/* 	for (uint32_t i = 0; i < 3; i++) { */
	/* 		brightness += 0xFF & (color >> 8*i); */
	/* 	} */
	/* 	const uint32_t threshold = 0x30; */
	/* 	if (brightness < threshold) { */
	/* 		uint32_t compensation = threshold / 3; */
	/* 		for (uint32_t i = 0; i < 3; i++) { */
	/* 			color += compensation << 8*i; */
	/* 		} */
	/* 	} */
	/* } */
	return color;// & 0xFFFFFF;
}


int l_common_gridColor(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 3) {
		error("`gridColor` accepts three arguments.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 1)) {
		error("`gridColor` accepts an integer as its first argument.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 2)) {
		error("`gridColor` accepts an integer as its second argument.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 3)) {
		error("`gridColor` accepts an integer as its third argument.", "");
		lua_error(l);
	}

	uint32_t x = lua_tointeger(l, 1);
	uint32_t y = lua_tointeger(l, 2);
	uint32_t z = lua_tointeger(l, 3);
	uint32_t v = gridColor(x, y, z);

	lua_pushinteger(l, v);

	return 1;
}

int l_common_perlin(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 3) {
		error("`perlin` accepts three arguments.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 1)) {
		error("`perlin` accepts an integer as its first argument.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 2)) {
		error("`perlin` accepts an integer as its second argument.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 3)) {
		error("`perlin` accepts an integer as its third argument.", "");
		lua_error(l);
	}

	uint32_t x = lua_tointeger(l, 1);
	uint32_t y = lua_tointeger(l, 2);
	uint32_t z = lua_tointeger(l, 3);
	uint32_t v;
	{
		const uint32_t gridScale = 40;
		const uint32_t perlinScale = 256;
		uint32_t gx = x / gridScale;
		uint32_t gy = y / gridScale;
		uint32_t gz = z / gridScale;
		int32_t fraction_x = (x - gridScale*gx);
		int32_t fraction_y = (y - gridScale*gy);
		int32_t fraction_z = (z - gridScale*gz);
		int32_t value = 0;
		for (uint8_t iz = 0; iz < 2; iz++) {
			for (uint8_t iy = 0; iy < 2; iy++) {
				for (uint8_t ix = 0; ix < 2; ix++) {
					int32_t offset_x = (int32_t) ((int64_t)x - (int64_t)(gridScale*(gx + ix)));
					int32_t offset_y = (int32_t) ((int64_t)y - (int64_t)(gridScale*(gy + iy)));
					int32_t offset_z = (int32_t) ((int64_t)z - (int64_t)(gridScale*(gz + iz)));
					uint64_t h = hash((((uint64_t) (gz + iz)) << 32) + (((uint64_t) (gy + iy)) << 16) + (gx + ix));
					int64_t normal_x = (int64_t) (h & 0xFFFF) - 0x8000;
					int64_t normal_y = (int64_t) ((h >> 16) & 0xFFFF) - 0x8000;
					int64_t normal_z = (int64_t) ((h >> 32) & 0xFFFF) - 0x8000;
					int64_t interpolated = ((ix ? fraction_x : gridScale - fraction_x)
											* (iy ? fraction_y : gridScale - fraction_y)
											* (iz ? fraction_z : gridScale - fraction_z));
					int64_t dot = offset_x * normal_x + offset_y * normal_y + offset_z * normal_z;
					value += (perlinScale/2 * dot * interpolated / 0x8000 / (int64_t)(gridScale/2) / ((int64_t)gridScale*(int64_t)gridScale*(int64_t)gridScale) / 3);
				}
			}
		}
		v = value + perlinScale/2;
	}

	lua_pushinteger(l, v);

	return 1;
}

int l_common_hash(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 1) {
		error("`hash` accepts one argument.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 1)) {
		error("`hash` accepts an integer as its argument.", "");
		lua_error(l);
	}
	lua_Unsigned k = lua_tointeger(l, 1);
	k = hash(k);
	(void) lua_pushinteger(l, k);

	return 1;
}

int l_common_hash3d(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 3) {
		error("`hash3d` accepts three arguments.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 1)) {
		error("`hash3d` accepts an integer as its first argument.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 2)) {
		error("`hash3d` accepts an integer as its second argument.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 3)) {
		error("`hash3d` accepts an integer as its third argument.", "");
		lua_error(l);
	}
	uint32_t x = lua_tointeger(l, 1);
	uint32_t y = lua_tointeger(l, 2);
	uint32_t z = lua_tointeger(l, 3);
	uint64_t k = hash3d(x, y, z);
	(void) lua_pushinteger(l, k);

	return 1;
}

int l_common_random(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 0) {
		error("`random` does not accept any arguments", "");
		lua_error(l);
	}

	(void) lua_pushinteger(l, rand());

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

int l_common_truncate(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 1) {
		error("`truncate` requires 1 argument", "");
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
		error("Argument of `truncate` must be a number.", "");
		lua_error(l);
	}

	int64_t i = s;

	(void) lua_pushinteger(l, i);

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

int l_common_atan2(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 2) {
		error("`atan2` requires 2 arguments", "");
		lua_error(l);
	}

	vec_t y;
	vec_t x;

	int arg_index = 1;
	if (lua_isinteger(l, arg_index)) {
		y = lua_tointeger(l, arg_index);
	}
	else if (lua_isnumber(l, arg_index)) {
		y = lua_tonumber(l, arg_index);
	}
	else {
		error("First argument of `atan2` must be a number.", "");
		lua_error(l);
	}

	arg_index++;
	if (lua_isinteger(l, arg_index)) {
		x = lua_tointeger(l, arg_index);
	}
	else if (lua_isnumber(l, arg_index)) {
		x = lua_tonumber(l, arg_index);
	}
	else {
		error("Second argument of `atan2` must be a number.", "");
		lua_error(l);
	}

#ifdef DOUBLE_VEC
	vec_t result = atan2(y, x);
#else
	vec_t result = atan2f(y, x);
#endif

	(void) lua_pushnumber(l, result);

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

int l_common_string_sub(lua_State *l) {
	int argc = lua_gettop(l);
	if (argc != 3) {
		error("Requires 3 arguments", "");
		lua_error(l);
	}

	if (!lua_isstring(l, 1)) {
		error("First argument must be a string.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 2)) {
		error("Second argument must be an integer.", "");
		lua_error(l);
	}
	if (!lua_isinteger(l, 3)) {
		error("Third argument must be an integer.", "");
		lua_error(l);
	}

	const char *string = lua_tostring(l, 1);
	const size_t string_length = strlen(string);
	const lua_Integer i = lua_tointeger(l, 2);
	const lua_Integer j = lua_tointeger(l, 3);
	if (i < 1) {
		error("Second argument must be greater than 0. Got %li\n", i);
		lua_error(l);
	}
	if (i > string_length+1) {
		error("Second argument must not be greater than the length of the string plus 1. Got %li\n", i);
		lua_error(l);
	}
	if (j < i-1) {
		error("Third argument must not be more than 1 less than the second argument. Got %li, %li\n", i, j);
		lua_error(l);
	}
	if (j > string_length) {
		error("Third argument must not be greater than the length of the string. Got %li\n", j);
		lua_error(l);
	}
	if ((i > string_length) || (j < i)) {
		(void) lua_pushlstring(l, &string[i-1], 0);
	}
	else {
		(void) lua_pushlstring(l, &string[i-1], (j) - (i-1));
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


#define string_toLower(c) (((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : c)

static bool string_isDigit(uint8_t character) {
	return (character >= '0') && (character <= '9');
}

static int string_toDouble(double *result, const uint8_t *string, const size_t string_length) {
	int e = ERR_OK;

	ptrdiff_t index = 0;
	bool tempBool;
	bool negative = false;
	ptrdiff_t power = 0;
	bool power_negative = false;

	*result = 0.0;

	if (string[index] == '-') {
		index++;

		if ((size_t) index >= string_length) {
			e = ERR_GENERIC;
			goto cleanup;
		}

		negative = true;
	}

	/* Try .1 */
	if (string[index] == '.') {
		index++;

		if ((size_t) index >= string_length) {
			e = ERR_GENERIC;
			goto cleanup;
		}

		power = 10;

		if (!string_isDigit(string[index])) {
			e = ERR_GENERIC;
			goto cleanup;
		}

		*result += (double) (string[index] - '0') / (double) power;

		index++;

		while (((size_t) index < string_length) && (string_toLower(string[index]) != 'e')) {
			power = 10 * power;

			tempBool = string_isDigit(string[index]);
			if (!tempBool) {
				e = ERR_GENERIC;
				goto cleanup;
			}

			*result += (double) (string[index] - '0') / (double) power;
			index++;
		}
	}
	// Try 1.2, 1., and 1
	else {
		if (!string_isDigit(string[index])) {
			e = ERR_GENERIC;
			goto cleanup;
		}

		*result = string[index] - '0';

		index++;

		while (((size_t) index < string_length)
		       && (string_toLower(string[index]) != 'e')
		       && (string[index] != '.')) {
			if (!string_isDigit(string[index])) {
				e = ERR_GENERIC;
				goto cleanup;
			}

			*result = *result * 10.0 + (double) (string[index] - '0');

			index++;
		}

		if (string[index] == '.') {
			index++;

			if ((size_t) index >= string_length) {
				// eError = duckLisp_error_push(duckLisp, DL_STR("Expected a digit after decimal point."), index);
				// e = eError ? eError : dl_error_bufferOverflow;
				// This is expected. 1. 234.e61  435. for example.
				goto cleanup;
			}

			power = 1;
		}

		while (((size_t) index < string_length) && (string_toLower(string[index]) != 'e')) {
			power = 10 * power;
			if (!string_isDigit(string[index])) {
				e = ERR_GENERIC;
				goto cleanup;
			}

			*result += (double) (string[index] - '0') / (double) power;

			index++;
		}
	}

	// …e3
	if (string_toLower(string[index]) == 'e') {
		index++;

		if ((size_t) index >= string_length) {
			e = ERR_GENERIC;
			goto cleanup;
		}

		if (string[index] == '-') {
			index++;

			if ((size_t) index >= string_length) {
				e = ERR_GENERIC;
				goto cleanup;
			}

			power_negative = true;
		}

		if (!string_isDigit(string[index])) {
			e = ERR_GENERIC;
			goto cleanup;
		}

		power = string[index] - '0';

		index++;

		while ((size_t) index < string_length) {
			if (!string_isDigit(string[index])) {
				e = ERR_GENERIC;
				goto cleanup;
			}

			power = power * 10 + string[index] - '0';

			index++;
		}

		if (power_negative) {
			if (power == 0) {
				e = ERR_GENERIC;
				goto cleanup;
			}

			for (ptrdiff_t i = 0; i < power; i++) {
				*result /= 10.0;
			}
		}
		else {
			for (ptrdiff_t i = 0; i < power; i++) {
				*result *= 10.0;
			}
		}
	}

	if ((size_t) index != string_length) {
		e = ERR_CRITICAL;
		goto cleanup;
	}

	if (negative) {
		*result = -*result;
	}

	cleanup:

	return e;
}

int l_common_parse_double(lua_State *l) {
	int error = ERR_CRITICAL;
	
	if (!lua_isstring(l, 1)) {
		error("Argument must be a string.", "");
		lua_error(l);
	}

	size_t string_length = 0;
	const uint8_t *string = (uint8_t *) lua_tolstring(l, 1, &string_length);
	double result = 0.0;
	error = string_toDouble(&result, string, string_length);
	if (error) {
		(void) lua_pushnil(l);
	}
	else {
		(void) lua_pushnumber(l, result);
	}
	return 1;
}
