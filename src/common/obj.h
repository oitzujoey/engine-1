
#ifndef OBJ_H
#define OBJ_H

#include <lua.h>
#include "common.h"
#include "str.h"

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
    void *children;
    int length;
} entity_t;

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


int l_loadObj(lua_State *Lua);

#endif
