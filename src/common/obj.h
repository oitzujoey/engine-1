
#ifndef OBJ_H
#define OBJ_H

#include <lua.h>
#include "common.h"
#include "str.h"

typedef struct {
    // string_t name;
    vec3_t *vertices;
    int vertices_length;
    int **faces;
    int faces_length;
    vec3_t *surface_normals;    // Same length as faces.
} model_t;

typedef struct {
	model_t *models;
	int models_length;
	int models_length_actual;
} modelList_t;

extern modelList_t g_modelList;

int obj_isValidModelIndex(int index);
int modelList_createModel(model_t **model, int *index);
void modelList_init(void);
void modelList_free(void);

void model_init(model_t *model);

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

void model_free(model_t *model);

int l_obj_loadOoliteDAT(lua_State *luaState);

int l_loadObj(lua_State *Lua);

#endif
