
#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <time.h>
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



/* cfg2.h */
/* ====== */

typedef enum cfg2_var_type_s {
	// none is usually used for commands.
	cfg2_var_type_none,
	cfg2_var_type_vector,
	cfg2_var_type_integer,
	cfg2_var_type_string
} cfg2_var_type_t;

/*
game is what the gamecode has access to.
administrator is what the console has access to.
An administrator can request to be elevated to supervisor. This will allow modification of engine-breaking variables.
*/
typedef enum {
	cfg2_admin_game,
	cfg2_admin_administrator,
	cfg2_admin_supervisor
} cfg2_admin_t;

typedef struct cfg2_var_s {
	char *name;
	vec_t vector;           // vec_t (float)
	int integer;            // int
	char *string;           // string
	// char *command;          // Command sent to variable
	cfg2_var_type_t type;   // Type of variable: none, vect, int, string
	cfg2_admin_t permissionRead;        // Permission level required to read
	cfg2_admin_t permissionWrite;       // Permission level required to write
	cfg2_admin_t permissionDelete;      // Permission level required to delete
	cfg2_admin_t permissionCallback;    // Permission level required to read and write callback.
	unsigned int frequency; // Frequency that the variable is used by the console. Higher values mean it is used more.
	int (*callback)(struct cfg2_var_s *var, const char *command);   // The callback.
} cfg2_var_t;

typedef struct {
	bool quit;
	cfg2_admin_t adminLevel;
	cfg2_var_t *vars;
	unsigned int vars_length;
	lua_State *luaState;
	unsigned int recursionDepth;
	unsigned int maxRecursion;
} cfg2_t;

typedef struct {
	char *name;
	vec_t vector;
	int integer;
	char *string;
	cfg2_var_type_t type;
	cfg2_admin_t permissionRead;
	cfg2_admin_t permissionWrite;
	cfg2_admin_t permissionDelete;
	cfg2_admin_t permissionCallback;
	int (*callback)(cfg2_var_t *var, const char *command);
} cfg2_var_init_t;


#endif
