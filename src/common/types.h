
#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <lua.h>

/* Vector math structures */
/* ====================== */

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

/* String structure */
/* ================ */

typedef struct {
    char *value;
    unsigned int length;
    unsigned int memsize;
} string_t;

/* Internal model structure */
/* ======================== */

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

typedef struct {
	model_t *models;
	int models_length;
	int models_length_actual;
} modelList_t;

/* Wavefront OBJ structure */
/* ======================= */

typedef struct {
    int geometric_vertex;
    int texture_vertex;
    int vertex_normal;
} face_t;

typedef struct {
    face_t *faces;
    int faces_length;
} faceset_t;

typedef struct {
    string_t object_name;
    string_t material_library;
    vec4_t *geometric_vertices;
    int geometric_vertices_length;
    vec3_t *texture_vertices;
    int texture_vertices_length;
    vec3_t *vertex_normals;
    int vertex_normals_length;
    string_t material_name;
    int smoothing_group;
    /* Note: Width will come before length. */
    faceset_t *facesets;
    int facesets_length;
} obj_t;


/* Wavefront MTL structure */
/* ======================= */

typedef struct {
    string_t material_name;
} mtl_t;

#endif
