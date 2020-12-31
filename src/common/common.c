
#include "common.h"
#include <stdio.h>
#include <math.h>

int error;

vfs_t g_vfs;

int l_puts(lua_State *Lua) {
    printf(COLOR_CYAN"Lua {"COLOR_NORMAL"%s"COLOR_CYAN"}"COLOR_NORMAL"\n", lua_tostring(Lua, 1));
    return 0;
}

int MSB(int n) {
	int i;
	
	for (i = 0; (1<<i) <= n; i++);
	
	return i - 1;
}

void vec3_copy(vec3_t *destination, vec3_t *source) {
	(*destination)[0] = (*source)[0];
	(*destination)[1] = (*source)[1];
	(*destination)[2] = (*source)[2];
}

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

void vec3_subtract(vec3_t *result, vec3_t *a, vec3_t *b) {
	(*result)[0] = (*a)[0] - (*b)[0];
	(*result)[1] = (*a)[1] - (*b)[1];
	(*result)[2] = (*a)[2] - (*b)[2];
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
