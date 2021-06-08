
#ifndef VECTOR_H
#define VECTOR_H

#include "types.h"

void vec3_copy(vec3_t *destination, vec3_t *source);
void vec3_dotProduct(vec_t *result, vec3_t *a, vec3_t *b);
void vec3_crossProduct(vec3_t *result, vec3_t *a, vec3_t *b);
void vec3_add(vec3_t *result, vec3_t *a, vec3_t *b);
void vec3_subtract(vec3_t *result, vec3_t *a, vec3_t *b);
int vec3_normalize(vec3_t *v);

void quat_copy(quat_t *out, quat_t *in);
void quat_hamilton(quat_t *out, quat_t *q0, quat_t *q1);
void quat_conjugate(quat_t *q);
vec_t quat_norm(quat_t *q);
int quat_normalize(quat_t *q);
void quat_inverse(quat_t *q);
#define quat_unitInverse(q) quat_conjugate(q)
void vec3_rotate(vec3_t *v, quat_t *q);
void quat_print(quat_t *q);

/* Lua */
/* === */

int l_vec3_crossProduct(lua_State *luaState);

int l_vec3_rotate(lua_State *luaState);

int l_hamiltonProduct(lua_State *luaState);
int l_quatNormalize(lua_State *luaState);

#endif
