
#include <SDL2/SDL.h>
#include <lua.h>

#ifndef TYPES_H
#define TYPES_H

#ifdef DOUBLE_VEC
typedef double vec_t;
#else
typedef float vec_t;
#endif
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

typedef struct {
	vec_t s;
	vec3_t v;
} quat_t;

typedef struct {
    // string_t name;
    vec3_t *vertices;
    int vertices_length;
    int **faces;
    int faces_length;
    vec3_t *surface_normals;    // Same length as faces.
#ifdef CLIENT
	// Add pure array of vertices and normals to save rendering time. The server should never need this.
	// Both arrays are 3 (*faces) * 3 (**faces) * facesLength.
	vec_t *glVertices;
	vec_t *glNormals;
#endif
} model_t;

#endif
