
#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <time.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <lua.h>
#include <enet/enet.h>
#ifdef CLIENT
#include <GL/glew.h>
#endif
#include <physfs.h>

#define MAX_CLIENTS         2


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


/* RMSH file structure */
typedef struct {
	// Should always be RMSH
	uint8_t magic[4];
	uint32_t version;

	vec3_t mins;
	vec3_t maxs;
	vec_t *vertices;  // Length: vertices_length
	vec_t *vertexNormals;  // Length: vertexNormals_length
	vec_t *vertexTextureCoords;  // Length: vertexTextureCoords_length
	uint32_t vertices_length;
	uint32_t vertexNormals_length;
	uint32_t vertexTextureCoords_length;
} file_rmsh_t;

/* CMSH file structure */
typedef struct {
	// Magic number should always be CMSH
	uint8_t magic[4];
	uint32_t version;

	vec3_t mins;
	vec3_t maxs;
	vec_t *vertices;  // Length: vertices_length
	uint32_t vertices_length;
} file_cmsh_t;


/* Internal model structure */
/* ======================== */

/* DO NOT send this over the network. */
typedef struct {
    // string_t name;
	vec3_t *vertices;  // Probably not needed.
	int vertices_length;  // Probably not needed.
	int **faces;  // Probably not needed.
	int faces_length;  // Probably not needed.
    vec3_t *surface_normals;    // Same length as faces.  // Probably not needed.

	// Collisions:
	vec3_t collision_bb_mins;
	vec3_t collision_bb_maxs;
	vec3_t collision_aabb_mins;
	vec3_t collision_aabb_maxs;
	vec_t *collision_vertices;
	size_t collision_vertices_length;

	// Rendering:
#ifdef CLIENT
	// Add pure array of vertices and normals to save rendering time. The server should never need this.
	// Both arrays are 3 (*faces) * 3 (**faces) * facesLength.
	vec_t *glVertices;
	size_t glVertices_length;
	vec_t *glNormals;
	size_t glNormals_length;
	vec_t boundingSphere;  // Eventually maybe replace with AABB?
	ptrdiff_t *defaultMaterials;
	ptrdiff_t defaultMaterials_index;
	vec2_t **texCoords;
	int *texCoords_textures;
	size_t numBindableMaterials;
	vec_t *glTexCoords;
	size_t glTexCoords_length;
#endif
} model_t;

typedef struct {
	model_t *models;
	int models_length;
	int models_length_actual;
} modelList_t;



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
	Uint32 maxFramerate;
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
	// Hierarchy:
	// Children are specified by index and type. They will all point to only models or only entities.
	ptrdiff_t *children;
	size_t children_length;

	// Type:
	entity_childType_t childType;

	// Physical state:
	vec_t scale;
	vec3_t position;
	quat_t orientation;

	// Collision detection:
	vec3_t mins;
	vec3_t maxs;
	bool disableCollisions;  // Disable collision checking for all entities and models below.

	// Memory management:
	bool inUse;

	// Rendering:
#ifdef CLIENT
	bool shown;
	ptrdiff_t *materials;
	ptrdiff_t materials_length;
#endif
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
	uint8_t *keyUpCommand;
	uint8_t *keyDownCommand;
} keybind_t;

typedef struct {
	keybind_t *keys;
	size_t length;
	uint8_t *mouseMotionCallbackName;
} keybinds_t;

/* network.h */
/* ========= */

typedef enum {
	network_lua_type_none,
	network_lua_type_integer,
	network_lua_type_real,
	network_lua_type_boolean,
	network_lua_type_string,
	network_lua_type_table
} network_lua_type_t;

/* shader.h */
/* ======== */

typedef struct {
	size_t shader_index;
	GLuint program;
	struct {
		GLint orientation;
		GLint position;
		GLint scale;
		GLint aspectRatio;
	} uniform;
	bool instanced;  // Enable GPU instancing for this shader.
} Shader;

/* material.h */
/* ========= */

typedef struct {
#ifdef CLIENT
	GLuint texture;
	Shader *shader;
	bool transparent;
	bool depthSort;
	bool cull;
#else
	// Make the compiler happy.
	bool dummy;
#endif
} material_t;

typedef struct {
	material_t *materials;
	size_t materials_length;
} material_list_t;

#endif // TYPES_H
