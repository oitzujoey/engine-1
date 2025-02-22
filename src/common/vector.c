
#include "vector.h"
#include <math.h>
#include <stdio.h>
#include "common.h"
#include "log.h"

void vec3_clear(vec3_t *destination) {
	(*destination)[0] = 0;
	(*destination)[1] = 0;
	(*destination)[2] = 0;
}

void vec3_copy(vec3_t *destination, vec3_t *source) {
	(*destination)[0] = (*source)[0];
	(*destination)[1] = (*source)[1];
	(*destination)[2] = (*source)[2];
}

/*
a: First input to dot product.
b: Second input to dot product.
returns: Dot product.
*/
void vec3_dotProduct(vec_t *result, vec3_t *a, vec3_t *b) {
	// Ewww!
	*result = (*a)[0]*(*b)[0] + (*a)[1]*(*b)[1] + (*a)[2]*(*b)[2];
}

void vec3_crossProduct(vec3_t *result, vec3_t *a, vec3_t *b) {
	// Ewww!
	(*result)[0] = (*a)[1]*(*b)[2] - (*a)[2]*(*b)[1];
	(*result)[1] = (*a)[2]*(*b)[0] - (*a)[0]*(*b)[2];
	(*result)[2] = (*a)[0]*(*b)[1] - (*a)[1]*(*b)[0];
}

void vec3_add(vec3_t *result, vec3_t *a, vec3_t *b) {
	(*result)[0] = (*a)[0] + (*b)[0];
	(*result)[1] = (*a)[1] + (*b)[1];
	(*result)[2] = (*a)[2] + (*b)[2];
}

void vec3_subtract(vec3_t *result, vec3_t *a, vec3_t *b) {
	(*result)[0] = (*a)[0] - (*b)[0];
	(*result)[1] = (*a)[1] - (*b)[1];
	(*result)[2] = (*a)[2] - (*b)[2];
}

vec_t vec3_norm2(const vec3_t *v) {
	return (*v)[0]*(*v)[0] + (*v)[1]*(*v)[1] + (*v)[2]*(*v)[2];
}

vec_t vec3_norm(const vec3_t *v) {
	return sqrt((*v)[0]*(*v)[0] + (*v)[1]*(*v)[1] + (*v)[2]*(*v)[2]);
}

int vec3_normalize(vec3_t *v) {
	int error = ERR_OK;

	vec_t magnitude = sqrt((*v)[0]*(*v)[0] + (*v)[1]*(*v)[1] + (*v)[2]*(*v)[2]);
	if (magnitude == 0.0) {
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	(*v)[0] /= magnitude;
	(*v)[1] /= magnitude;
	(*v)[2] /= magnitude;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

void quat_copy(quat_t *out, quat_t *in) {
	*out = *in;
}

void quat_hamilton(quat_t *out, quat_t *q0, quat_t *q1) {
	quat_t temp;
	temp.s    = (q0->s * q1->s   ) - (q0->v[0] * q1->v[0]) - (q0->v[1] * q1->v[1]) - (q0->v[2] * q1->v[2]);
	temp.v[0] = (q0->s * q1->v[0]) + (q0->v[0] * q1->s   ) + (q0->v[1] * q1->v[2]) - (q0->v[2] * q1->v[1]);
	temp.v[1] = (q0->s * q1->v[1]) - (q0->v[0] * q1->v[2]) + (q0->v[1] * q1->s   ) + (q0->v[2] * q1->v[0]);
	temp.v[2] = (q0->s * q1->v[2]) + (q0->v[0] * q1->v[1]) - (q0->v[1] * q1->v[0]) + (q0->v[2] * q1->s   );
	*out = temp;
}

void quat_conjugate(quat_t *q) {
	q->v[0] = -q->v[0];
	q->v[1] = -q->v[1];
	q->v[2] = -q->v[2];
}

vec_t quat_norm(quat_t *q) {
	return sqrt(q->s * q->s + q->v[0] * q->v[0] + q->v[1] * q->v[1] + q->v[2] * q->v[2]);
}

int quat_normalize(quat_t *q) {
	int error = ERR_OK;

	vec_t magnitude = sqrt(q->s * q->s + q->v[0] * q->v[0] + q->v[1] * q->v[1] + q->v[2] * q->v[2]);
	if (magnitude == 0.0) {
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	q->s    = q->s    / magnitude;
	q->v[0] = q->v[0] / magnitude;
	q->v[1] = q->v[1] / magnitude;
	q->v[2] = q->v[2] / magnitude;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

void quat_inverse(quat_t *q) {
	vec_t norm2 = q->s * q->s + q->v[0] * q->v[0] + q->v[1] * q->v[1] + q->v[2] * q->v[2];
	q->s    = q->s    / norm2;
	q->v[0] = q->v[0] / norm2;
	q->v[1] = q->v[1] / norm2;
	q->v[2] = q->v[2] / norm2;
}

// Assume q is a unit quaternion.
void vec3_rotate(vec3_t *v, quat_t *q) {
	quat_t vector;
	quat_t qInverse;
	
	vector.s = 0;
	vec3_copy(&vector.v, v);
	
	quat_copy(&qInverse, q);
	quat_unitInverse(&qInverse);
	
	quat_hamilton(&vector, q, &vector);
	quat_hamilton(&vector, &vector, &qInverse);
	
	vec3_copy(v, &vector.v);
}

void quat_print(quat_t *q) {
	printf("quat_t {%f, %f, %f, %f}\n", q->s, q->v[0], q->v[1], q->v[2]);
}

/* Lua */
/* === */

/* l_vec3_crossProduct
a0: vec3_t vector0
a1: vec3_t vector1
r0: vec3_t crossProduct
*/
int l_vec3_crossProduct(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	vec3_t v0, v1, result;
	
	lua_pushstring(luaState, "x");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key x must be a number", "");
		lua_error(luaState);
	}
	v0[0] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "y");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key y must be a number", "");
		lua_error(luaState);
	}
	v0[1] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "z");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key z must be a number", "");
		lua_error(luaState);
	}
	v0[2] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	
	// lua_pushstring(luaState, "w");
	// if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
	// 	error("Key w must be a number", "");
	// 	lua_error(luaState);
	// }
	// q.s = lua_tonumber(luaState, -1);
	// lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "x");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key x must be a number", "");
		lua_error(luaState);
	}
	v1[0] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "y");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key y must be a number", "");
		lua_error(luaState);
	}
	v1[1] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "z");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key z must be a number", "");
		lua_error(luaState);
	}
	v1[2] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	
	result[0] = v0[1]*v1[2] - v0[2]*v1[1];
	result[1] = v0[2]*v1[0] - v0[0]*v1[2];
	result[2] = v0[0]*v1[1] - v0[1]*v1[0];
	
	
	error = ERR_OK;
	
	lua_newtable(luaState);
	
	lua_pushliteral(luaState, "x");
	lua_pushnumber(luaState, result[0]);
	lua_settable(luaState, -3);

	lua_pushliteral(luaState, "y");
	lua_pushnumber(luaState, result[1]);
	lua_settable(luaState, -3);

	lua_pushliteral(luaState, "z");
	lua_pushnumber(luaState, result[2]);
	lua_settable(luaState, -3);


	lua_pushinteger(luaState, error);

	return 2;
}


/* l_vec3_rotate
a0: vec3_t vector
a1: quat_t orientation
r0: vec3_t rotatedVector
*/
int l_vec3_rotate(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	quat_t q, qInverse, v;
	
	// lua_pushstring(luaState, "w");
	// if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
	// 	error("Key w must be a number", "");
	// 	lua_error(luaState);
	// }
	// q0.s = lua_tonumber(luaState, -1);
	// lua_pop(luaState, 1);
	
	v.s = 0;
	
	lua_pushstring(luaState, "x");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key x must be a number", "");
		lua_error(luaState);
	}
	v.v[0] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "y");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key y must be a number", "");
		lua_error(luaState);
	}
	v.v[1] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "z");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key z must be a number", "");
		lua_error(luaState);
	}
	v.v[2] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	
	lua_pushstring(luaState, "w");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key w must be a number", "");
		lua_error(luaState);
	}
	q.s = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "x");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key x must be a number", "");
		lua_error(luaState);
	}
	q.v[0] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "y");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key y must be a number", "");
		lua_error(luaState);
	}
	q.v[1] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "z");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key z must be a number", "");
		lua_error(luaState);
	}
	q.v[2] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	
	quat_copy(&qInverse, &q);
	quat_unitInverse(&qInverse);
	
	quat_hamilton(&v, &q, &v);
	quat_hamilton(&v, &v, &qInverse);
	
	
	error = ERR_OK;
	
	lua_newtable(luaState);
	
	// lua_pushstring(luaState, "w");
	// lua_pushnumber(luaState, v.s);
	// lua_settable(luaState, -3);
	
	lua_pushliteral(luaState, "x");
	lua_pushnumber(luaState, v.v[0]);
	lua_settable(luaState, -3);

	lua_pushliteral(luaState, "y");
	lua_pushnumber(luaState, v.v[1]);
	lua_settable(luaState, -3);

	lua_pushliteral(luaState, "z");
	lua_pushnumber(luaState, v.v[2]);
	lua_settable(luaState, -3);


	lua_pushinteger(luaState, error);

	return 2;
}

int l_hamiltonProduct(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	quat_t q0, q1, result;
	
	lua_pushstring(luaState, "w");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key w must be a number", "");
		lua_error(luaState);
	}
	q0.s = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "x");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key x must be a number", "");
		lua_error(luaState);
	}
	q0.v[0] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "y");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key y must be a number", "");
		lua_error(luaState);
	}
	q0.v[1] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "z");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key z must be a number", "");
		lua_error(luaState);
	}
	q0.v[2] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	
	lua_pushstring(luaState, "w");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key w must be a number", "");
		lua_error(luaState);
	}
	q1.s = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "x");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key x must be a number", "");
		lua_error(luaState);
	}
	q1.v[0] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "y");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key y must be a number", "");
		lua_error(luaState);
	}
	q1.v[1] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "z");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key z must be a number", "");
		lua_error(luaState);
	}
	q1.v[2] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	quat_hamilton(&result, &q0, &q1);
	
	
	error = ERR_OK;
	
	lua_newtable(luaState);
	
	lua_pushstring(luaState, "w");
	lua_pushnumber(luaState, result.s);
	lua_settable(luaState, -3);
	
	lua_pushliteral(luaState, "x");
	lua_pushnumber(luaState, result.v[0]);
	lua_settable(luaState, -3);

	lua_pushliteral(luaState, "y");
	lua_pushnumber(luaState, result.v[1]);
	lua_settable(luaState, -3);

	lua_pushliteral(luaState, "z");
	lua_pushnumber(luaState, result.v[2]);
	lua_settable(luaState, -3);


	lua_pushinteger(luaState, error);

	return 2;
}

int l_quatNormalize(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	quat_t q;
	
	lua_pushstring(luaState, "w");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key w must be a number", "");
		lua_error(luaState);
	}
	q.s = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "x");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key x must be a number", "");
		lua_error(luaState);
	}
	q.v[0] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "y");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key y must be a number", "");
		lua_error(luaState);
	}
	q.v[1] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "z");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key z must be a number", "");
		lua_error(luaState);
	}
	q.v[2] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	
	
	quat_normalize(&q);
	
	
	error = ERR_OK;
	
	lua_newtable(luaState);
	
	lua_pushstring(luaState, "w");
	lua_pushnumber(luaState, q.s);
	lua_settable(luaState, -3);
	
	lua_pushliteral(luaState, "x");
	lua_pushnumber(luaState, q.v[0]);
	lua_settable(luaState, -3);

	lua_pushliteral(luaState, "y");
	lua_pushnumber(luaState, q.v[1]);
	lua_settable(luaState, -3);

	lua_pushliteral(luaState, "z");
	lua_pushnumber(luaState, q.v[2]);
	lua_settable(luaState, -3);


	lua_pushinteger(luaState, error);

	return 2;
}

// Normalize the vector part of an axis angle.
int l_aaNormalize(lua_State *luaState) {
	int error = ERR_OK;

	// This is an axis angle. Trust me.
	quat_t q;

	lua_pushstring(luaState, "w");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key w must be a number", "");
		lua_error(luaState);
	}
	q.s = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, "x");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key x must be a number", "");
		lua_error(luaState);
	}
	q.v[0] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, "y");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key y must be a number", "");
		lua_error(luaState);
	}
	q.v[1] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, "z");
	if (lua_gettable(luaState, 1) != LUA_TNUMBER) {
		error("Key z must be a number", "");
		lua_error(luaState);
	}
	q.v[2] = lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);

	error = vec3_normalize(&q.v);
	if (error) {
		error("aaNormalize failed.", "");
		lua_error(luaState);
	}

	lua_newtable(luaState);

	lua_pushstring(luaState, "w");
	lua_pushnumber(luaState, q.s);
	lua_settable(luaState, -3);

	lua_pushliteral(luaState, "x");
	lua_pushnumber(luaState, q.v[0]);
	lua_settable(luaState, -3);

	lua_pushliteral(luaState, "y");
	lua_pushnumber(luaState, q.v[1]);
	lua_settable(luaState, -3);

	lua_pushliteral(luaState, "z");
	lua_pushnumber(luaState, q.v[2]);
	lua_settable(luaState, -3);

	lua_pushinteger(luaState, error);

	return 2;
}
