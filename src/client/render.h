
#ifndef RENDER_H
#define RENDER_H

#include "lua.h"
#include "../common/cfg2.h"
#include "../common/types.h"
#include "../common/array.h"

extern SDL_DisplayMode g_displayMode;
extern char *g_openglLogFileName;

const char *render_glGetErrorString(GLenum glError);
void render_logShaderInfo(GLuint shaderIndex);
void render_logProgramInfo(GLuint programIndex);
int render_callback_updateLogFileName(cfg2_var_t *var);

int render_initOpenGL(void);
int render(entity_t *entity);


// Each of these objects should only exist for a single frame.
typedef struct {
	size_t glVertices_length;
	vec_t *glVertices;
	size_t glNormals_length;
	vec_t *glNormals;
	size_t glTexCoords_length;
	vec_t *glTexCoords;
	quat_t orientation;
	vec3_t position;
	vec_t scale;
	material_t *material;
} renderObject_t;

typedef struct {
	size_t glVertices_length;
	vec_t *glVertices;
	size_t glNormals_length;
	vec_t *glNormals;
	size_t glTexCoords_length;
	vec_t *glTexCoords;
	material_t *material;
	// Unique per instance:
	array_t orientations;
	array_t positions;
	array_t scales;
} InstancedRenderObjects;

typedef struct {
	size_t shader_index;
	size_t model_index;
	array_t instancedRenderObjects;
} InstancedRenderObjectArray;

#endif // RENDER_H
