
#ifndef RENDER_H
#define RENDER_H

#include "lua.h"
#include "../common/cfg2.h"
#include "../common/types.h"

extern SDL_DisplayMode g_displayMode;
extern char *g_openglLogFileName;

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
	ptrdiff_t material_index;
	GLuint g_shaderProgram;
} renderObject_t;

#endif // RENDER_H
