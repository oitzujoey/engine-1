
#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <lua.h>
#include <enet/enet.h>

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


/* Internal model structure */
/* ======================== */

/* DO NOT send this over the network. */
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
    char *object_name;
    char *material_library;
    vec4_t *geometric_vertices;
    int geometric_vertices_length;
    vec3_t *texture_vertices;
    int texture_vertices_length;
    vec3_t *vertex_normals;
    int vertex_normals_length;
    char *material_name;
    int smoothing_group;
    /* Note: Width will come before length. */
    faceset_t *facesets;
    int facesets_length;
} obj_t;


/* Wavefront MTL structure */
/* ======================= */

typedef struct {
    char *material_name;
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
	int (*callback)(struct cfg2_var_s *var, const char *command, lua_State *luaState);   // The callback.
	char *script;
} cfg2_var_t;

typedef struct {
	bool quit;
	cfg2_admin_t adminLevel;
	cfg2_admin_t adminLevelDisguise;
	cfg2_var_t *vars;
	unsigned int vars_length;
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
	int (*callback)(cfg2_var_t *var, const char *command, lua_State *luaState);
} cfg2_var_init_t;


/* lua.h */
/* ===== */

typedef struct {
    lua_CFunction func;
    char *name;
} luaCFunc_t;

typedef struct {
	lua_State *luaState;
	const char *functionName;
} luaTimeout_t;


/* vfs.h */
/* ===== */

typedef struct {
	char **filenames;
	char **files;
	unsigned int files_length;
} vfs_mod_t;

typedef struct {
	vfs_mod_t *mods;
	unsigned int mods_length;
} vfs_mods_t;

typedef enum vfs_type_e {
	vfs_type_directory,
	vfs_type_zip
} vfs_type_t;

typedef struct {
	char *path;
	vfs_type_t workspace_type;
} vfs_t;

/* entity.h */
/* ======== */

typedef enum entity_childType_e {
	entity_childType_none,
	entity_childType_entity,
	entity_childType_model
} entity_childType_t;

typedef struct {
	// Children are specified by index and type.
	ptrdiff_t *children;
	size_t children_length;
	entity_childType_t childType;
	vec3_t position;
	quat_t orientation;
	bool inUse;
} entity_t;

typedef struct {
	entity_t *entities;
	size_t entities_length;
	/* entities_length_allocated always equals entities_length +
	   deletedEntities_length, so it is not really needed. */
	// int entities_length_allocated;
	ptrdiff_t *deletedEntities;
	size_t deletedEntities_length;
	size_t deletedEntities_length_allocated;
} entityList_t;

/* input.h */
/* ======= */

typedef enum {
	buttonType_keyboard,
	buttonType_mouse,
	buttonType_joystick,
	buttonType_controller
} buttonType_t;

typedef struct {
	buttonType_t buttonType;
	union {
		SDL_Keycode keycode;
		Uint8 mouseButton;
		Uint8 joystickButton;
		Uint8 controllerButton;
	} key;
	SDL_JoystickID which;
	char *keyUpCommand;
	char *keyDownCommand;
} keybind_t;

typedef struct {
	keybind_t *keys;
	size_t length;
} keybinds_t;

/* network.h */
/* ========= */

typedef enum {
	none,
	integer,
	real,
	boolean,
	string,
	table
} network_lua_type_t;

#endif
