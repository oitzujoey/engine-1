
#include "render.h"
#include <stdio.h>
#include <stdarg.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <math.h>
#include "../common/common.h"
#include "../common/log.h"
#include "../common/entity.h"
#include "../common/obj.h"
#include "../common/vector.h"
#include "../common/arena.h"
#include "../common/array.h"

extern material_list_t g_materialList;

SDL_Window* g_window;
SDL_DisplayMode g_displayMode;
SDL_GLContext g_GLContext;
GLuint g_VertexVbo;
GLuint g_colorVbo;
GLuint g_texCoordVbo;
GLint g_orientationUniform;
GLint g_positionUniform;
GLint g_screenHeightUniform;
GLint g_scaleUniform;
GLuint g_vao;
GLuint g_shaderProgram[2];
char *g_openglLogFileName;
float g_points[] = {
	-0.5, -0.5, 0,
	0.5, -0.5, 0,
	0, 0.5, 0
};
Allocator g_renderObjectArena;

array_t g_transparencyRenderObjects;


static const char *render_glGetErrorString(GLenum glError) {
	switch (glError) {
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	default:
		return "Bad error code";
	}
}

void render_logShaderInfo(GLuint shaderIndex) {
	const int maxLength = 2048;
	int length = 0;
	char shaderLog[maxLength];
	glGetShaderInfoLog(shaderIndex, maxLength, &length, shaderLog);
	warning("Log for shader %i:\n%s", shaderIndex, shaderLog);
}

void render_logProgramInfo(GLuint programIndex) {
	const int maxLength = 2048;
	int length = 0;
	char programLog[maxLength];
	glGetProgramInfoLog(programIndex, maxLength, &length, programLog);
	warning("Log for shader %i:\n%s", programIndex, programLog);
}

int render_callback_updateLogFileName(cfg2_var_t *var) {
	g_openglLogFileName = var->string;
	return ERR_OK;
}

// static int openglWriteLog(const char *message, ...) {
// 	int error = ERR_CRITICAL;

// 	va_list arguments;
// 	FILE *file = fopen(g_openglLogFileName, "a");
// 	if (file == NULL) {
// 		error("Could not open \"%s\" for appending.", g_openglLogFileName);
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
	
// 	va_start(arguments, message);
// 	vfprintf(file, message, arguments);
// 	va_end(arguments);
	
// 	fclose(file);
	
// 	error = ERR_OK;
// 	cleanup_l:
// 	return error;
// }

int render_initOpenGL(void) {
	int error = ERR_CRITICAL;
	GLenum glError = GL_NO_ERROR;

	info("Initializing OpenGL.", "");
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG, SDL_TRUE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	
	g_GLContext = SDL_GL_CreateContext(g_window);
	if (g_GLContext == NULL) {
		critical_error("Could not create OpenGL context. SDL error: %s", SDL_GetError());
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	glewExperimental = GL_TRUE;
	glError = glewInit();
	if (glError != GLEW_OK) {
		critical_error("Could not initialize GLEW. SDL error: %s", SDL_GetError());
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	/* Brag about GPU capabilities. */
	
	const GLubyte *renderer = glGetString(GL_RENDERER);
	info("Renderer: %s", renderer);
	
	const GLubyte *version = glGetString(GL_VERSION);
	info("Version: %s", version);
	
	const GLenum parameters[] = {
		GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
		GL_MAX_CUBE_MAP_TEXTURE_SIZE,
		GL_MAX_DRAW_BUFFERS,
		GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
		GL_MAX_TEXTURE_IMAGE_UNITS,
		GL_MAX_TEXTURE_SIZE,
		GL_MAX_VARYING_FLOATS,
		GL_MAX_VERTEX_ATTRIBS,
		GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
		GL_MAX_VERTEX_UNIFORM_COMPONENTS,
		GL_MAX_VIEWPORT_DIMS,
		GL_STEREO,
	};
	const char* names[] = {
		"GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
		"GL_MAX_CUBE_MAP_TEXTURE_SIZE",
		"GL_MAX_DRAW_BUFFERS",
		"GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
		"GL_MAX_TEXTURE_IMAGE_UNITS",
		"GL_MAX_TEXTURE_SIZE",
		"GL_MAX_VARYING_FLOATS",
		"GL_MAX_VERTEX_ATTRIBS",
		"GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
		"GL_MAX_VERTEX_UNIFORM_COMPONENTS",
		"GL_MAX_VIEWPORT_DIMS",
		"GL_STEREO",
	};
	
	int values[2];
	for (int i = 0; i < 10; i++) {
		glGetIntegerv(parameters[i], &values[0]);
		glError = glGetError();
		if (glError) {
			error("glGetIntegerv returned with errors.", "");
			while (glError) {
				error("OpenGL error: %s", render_glGetErrorString(glError));
				glError = glGetError();
			}
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		info("%s %i", names[i], values[0]);
	}
	
	glGetIntegerv(parameters[10], values);
	glError = glGetError();
	if (glError) {
		error("glGetIntegerv returned with errors.", "");
		while (glError) {
			error("OpenGL error: %s", render_glGetErrorString(glError));
			glError = glGetError();
		}
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	info("%s %i %i", names[10], values[0], values[1]);
	
	glGetIntegerv(parameters[11], &values[0]);
	glError = glGetError();
	if (glError) {
		error("glGetIntegerv returned with errors.", "");
		while (glError) {
			error("OpenGL error: %s", render_glGetErrorString(glError));
			glError = glGetError();
		}
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	info("%s %i", names[11], values[0]);
	
	/* Turn on some optimizations. */
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// glDepthRange(0, 1);
	
	/* Setup shader variables. */
	
	g_VertexVbo = 0;
	glGenBuffers(1, &g_VertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_VertexVbo);
	
	g_colorVbo = 0;
	glGenBuffers(1, &g_colorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
	
	g_texCoordVbo = 0;
	glGenBuffers(1, &g_texCoordVbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_texCoordVbo);
	
	
	g_vao = 0;
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, g_VertexVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glBindBuffer(GL_ARRAY_BUFFER, g_texCoordVbo);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	
	/* Shaders */
	
	const char* vertexShaderSource0 =
		"#version 400\n"
		"layout(location = 0) in vec3 vp;\n"
		"layout(location = 1) in vec3 normal;\n"
		"layout(location = 2) in vec2 texCoord;\n"
		"uniform vec4 orientation;\n"
		"uniform vec3 position;\n"
		"uniform float screenHeight;\n"
		"uniform float scale;\n"
		"out vec3 color;\n"
		"out vec2 textureCoordinate;\n"
		
		"vec4 conjugate(vec4 a) {\n"
		"  return vec4(\n"
		"    -a.x,\n"
		"    -a.y,\n"
		"    -a.z,\n"
		"    a.w\n"
		"  );\n"
		"}\n"
		
		"vec4 hamilton(vec4 a, vec4 b) {\n"
		"  return vec4(\n"
		"    a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,\n"
		"    a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,\n"
		"    a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,\n"
		"    a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z\n"
		"  );\n"
		"}\n"
		
		"vec3 rotate(vec3 v, vec4 q) {\n"
		"  vec4 v4 = vec4(v, 0);\n"
		"  v4 = hamilton(hamilton(q, v4), conjugate(q));\n"
		"  return vec3(v4.x, v4.y, v4.z);\n"
		"}\n"

		"mat4 projectionMatrix = mat4("
		"  3.0/screenHeight, 0.0, 0.0, 0.0,"
		"  0.0, 3.0,  0.0,      0.0,"
		"  0.0, 0.0,          -1.01, 2.01,"
		"  0.0, 0.0,          -1.0, 0.0"
		");"
		
		"void main() {\n"
		"  vec3 vertex;\n"
		"  vertex = rotate(scale * vp, orientation);\n"
		"  vertex += position;\n"
		"  vec4 glPosition = projectionMatrix * vec4(vertex, 1.0);"
		"  gl_Position = glPosition;\n"
		"  float powerFactor = 1.01;"
		"  color = pow(powerFactor, -abs(glPosition.z))/powerFactor * rotate(normal, orientation);\n"
		"  textureCoordinate = texCoord;\n"
		"}\n";
	
	const char* fragmentShaderSource0 =
		"#version 400\n"
		"out vec4 frag_colour;"
		"in vec3 color;"
		"in vec2 textureCoordinate;"
		"uniform sampler2D ourTexture;"
		"void main() {"
		"  float dot = dot(color, vec3(0.0, 0.0, 1.0));"
		"  float mixing = 0.75;"
		"  dot = abs(dot) * (1.0 - mixing) + mixing;"
		"  frag_colour = texture(ourTexture, textureCoordinate) * vec4(dot, dot, dot, 1.0);"
		"}";
	
	/* Compile shaders and link program. */
	
	for (int i = 0; i < 2; i++) {
		g_shaderProgram[i] = glCreateProgram();
		if (g_shaderProgram[i] == 0) {
			error("glCreateProgram returned with errors.", "");
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
	}
	
	GLint compileStatus;
	
	GLuint vertexShader0 = glCreateShader(GL_VERTEX_SHADER);
	if (vertexShader0 == 0) {
		glError = glGetError();
		error("glCreateShader returned with errors.", "");
		while (glError) {
			error("OpenGL error: %s", render_glGetErrorString(glError));
			glError = glGetError();
		}
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	glShaderSource(vertexShader0, 1, &vertexShaderSource0, NULL);
	glCompileShader(vertexShader0);
	glGetShaderiv(vertexShader0, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		error("Vertex shader failed to compile.", "");
		render_logShaderInfo(vertexShader0);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	GLuint fragmentShader0 = glCreateShader(GL_FRAGMENT_SHADER);
	if (fragmentShader0 == 0) {
		glError = glGetError();
		error("glCreateShader returned with errors.", "");
		while (glError) {
			error("OpenGL error: %s", render_glGetErrorString(glError));
			glError = glGetError();
		}
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	glShaderSource(fragmentShader0, 1, &fragmentShaderSource0, NULL);
	glCompileShader(fragmentShader0);
	glGetShaderiv(fragmentShader0, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		error("Fragment shader failed to compile.", "");
		render_logShaderInfo(fragmentShader0);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	glAttachShader(g_shaderProgram[0], vertexShader0);
	glAttachShader(g_shaderProgram[0], fragmentShader0);
	glLinkProgram(g_shaderProgram[0]);
	glGetProgramiv(g_shaderProgram[0],  GL_LINK_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		error("Program failed to link.", "");
		render_logProgramInfo(g_shaderProgram[0]);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	/* Find uniform variable locations. */
	
	g_orientationUniform = glGetUniformLocation(g_shaderProgram[0], "orientation");
	if (g_orientationUniform < 0) {
		critical_error("Could not get uniform from program.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	g_positionUniform = glGetUniformLocation(g_shaderProgram[0], "position");
	if (g_positionUniform < 0) {
		critical_error("Could not get uniform from program.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	g_screenHeightUniform = glGetUniformLocation(g_shaderProgram[0], "screenHeight");
	if (g_screenHeightUniform < 0) {
		critical_error("Could not get uniform from program.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}

	g_scaleUniform = glGetUniformLocation(g_shaderProgram[0], "scale");
	if (g_scaleUniform < 0) {
		critical_error("Could not get uniform from program.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}

	/* Set background color. */
	
	glClearColor(0.1, 0.1, 0.1, 1.0);
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

// Note: `position` is `vec3_t *` and not `vec3_t` because `vec3_t` it decayed to a pointer. I'm not entirely clear on
// why this happens. It's possible that arrays always decay when passed to functions.
int renderModels(entity_t *entity, vec3_t *position, quat_t orientation, vec_t scale, ptrdiff_t material_index) {
	int e = ERR_OK;

	// TODO: Only allow one model in a model entity.
	// For each model...
	size_t children_length = entity->children_length;
	ptrdiff_t *children = entity->children;
	for (int j = 0; j < children_length; j++) {
		int modelIndex = children[j];
		model_t *model = &g_modelList.models[modelIndex];
		// TODO: Change this from texture #1 to whatever texture is called for.
		if (material_index < 0) material_index = model->defaultMaterials[0];
		material_t *material = &g_materialList.materials[material_index];

		// TODO: Fix frustum culling.
		/* // Make sure model is inside the frustum. */
		/* const vec_t fov = 1.0/120.0; */
		/* printf("j %i\n", j); */
		/* printf("position[2] %f\n", position[2]); */
		/* printf("model.boundingSphere %f\n", model.boundingSphere); */
		/* printf("scale %f\n", scale); */
		/* const vec_t localBoundingSphere = model.boundingSphere * scale; */
		/* printf("localBoundingSphere %f\n", localBoundingSphere); */
		/* if ((position[2] + localBoundingSphere < 0.0f) */
		/*     || (((fabs(fabs(position[0]) - localBoundingSphere)/position[2] > 16.0*fov) */
		/*          && (fabs(position[0])/position[2] > 16.0*fov)) */
		/*         || ((fabs(fabs(position[1]) - localBoundingSphere)/position[2] > 9.0*fov) */
		/*             && (fabs(position[1])/position[2] > 9.0*fov))) */
		/* ) { */
		/* 	printf("CONTINUE %i\n", modelIndex); */
		/* 	continue; */
		/* } */

		if (material->transparent) {
			// Defer rendering to the transparency pass.
			renderObject_t *ro = NULL;
			e = g_renderObjectArena.alloc(g_renderObjectArena.context, (void **) &ro, sizeof(renderObject_t));
			if (e) return e;
			ro->glVertices_length = model->glVertices_length;
			ro->glVertices = model->glVertices;
			ro->glNormals_length = model->glNormals_length;
			ro->glNormals = model->glNormals;
			ro->glTexCoords_length = model->glTexCoords_length;
			ro->glTexCoords = model->glTexCoords;
			(void) quat_copy(&ro->orientation, &orientation);
			(void) vec3_copy(&ro->position, position);
			ro->scale = scale;
			ro->material = material;
			ro->shaderProgram = g_shaderProgram[0];

			return array_push(&g_transparencyRenderObjects, &ro);
		}
		
		/* Render */
		
		glBindBuffer(GL_ARRAY_BUFFER, g_VertexVbo);
		glBufferData(GL_ARRAY_BUFFER, model->glVertices_length * sizeof(float), model->glVertices, GL_DYNAMIC_DRAW);
		
		glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
		glBufferData(GL_ARRAY_BUFFER, model->glNormals_length * sizeof(float), model->glNormals, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, g_texCoordVbo);
		glBufferData(GL_ARRAY_BUFFER, model->glTexCoords_length * sizeof(float), model->glTexCoords, GL_DYNAMIC_DRAW);

		glUniform4f(g_orientationUniform, 
			orientation.v[0],
			orientation.v[1],
			orientation.v[2],
			orientation.s
		);
		glUniform3f(g_positionUniform, 
		            (*position)[0],
		            (*position)[1],
		            (*position)[2]);
		glUniform1f(g_scaleUniform, scale);
		
		glUseProgram(g_shaderProgram[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, material->texture);
		glBindVertexArray(g_vao);
		// glVertices_length is always a multiple of three.
		glDrawArrays(GL_TRIANGLES, 0, model->glVertices_length / 3);
	}
	
	return e;
}

int renderEntity(entity_t *entity, vec3_t *position, quat_t *orientation, vec_t scale, ptrdiff_t material_index) {
	int error = ERR_CRITICAL;

	vec3_t localPosition;
	quat_t localOrientation;
	vec_t localScale;
	ptrdiff_t local_material_index;
	
	if (!entity->inUse) {
		// Don't draw deleted entities.
		warning("Attempted to draw deleted entity %i.", (int) (entity - g_entityList.entities));
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (!entity->shown) {
		// This is done client side because the entity doesn't contain any useful information.
		error = ERR_OK;
		goto cleanup_l;
	}

	vec3_copy(&localPosition, &entity->position);
	vec3_rotate(&localPosition, orientation);
	vec3_add(&localPosition, position, &localPosition);
	quat_hamilton(&localOrientation, orientation, &entity->orientation);
	localScale = scale * entity->scale;
	if (material_index < 0 && entity->materials_length) material_index = entity->materials[0];

	if (entity->childType == entity_childType_model) {
		error = renderModels(entity, &localPosition, localOrientation, localScale, material_index);
	}
	else if (entity->childType == entity_childType_entity) {
		size_t children_length = entity->children_length;
		ptrdiff_t *children = entity->children;
		for (int i = 0; i < children_length; i++) {
			error = renderEntity(&g_entityList.entities[children[i]],
			                     &localPosition,
			                     &localOrientation,
			                     localScale,
			                     material_index);
		}
	}
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

void renderRenderObject(renderObject_t *renderObject) {
	// We don't do frustum culling here because we should have already done it in the tree traversal.

	/* Render */

	glBindBuffer(GL_ARRAY_BUFFER, g_VertexVbo);
	glBufferData(GL_ARRAY_BUFFER, renderObject->glVertices_length * sizeof(float), renderObject->glVertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
	glBufferData(GL_ARRAY_BUFFER, renderObject->glNormals_length * sizeof(float), renderObject->glNormals, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, g_texCoordVbo);
	glBufferData(GL_ARRAY_BUFFER, renderObject->glTexCoords_length * sizeof(float), renderObject->glTexCoords, GL_DYNAMIC_DRAW);

	glUniform4f(g_orientationUniform,
				renderObject->orientation.v[0],
				renderObject->orientation.v[1],
				renderObject->orientation.v[2],
				renderObject->orientation.s
				);
	glUniform3f(g_positionUniform,
				renderObject->position[0],
				renderObject->position[1],
				renderObject->position[2]);
	glUniform1f(g_scaleUniform, renderObject->scale);

	glUseProgram(renderObject->shaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderObject->material->texture);
	glBindVertexArray(g_vao);
	// glVertices_length is always a multiple of three.
	glDrawArrays(GL_TRIANGLES, 0, renderObject->glVertices_length / 3);
}

int renderRenderObjects(array_t *renderObjects) {
	size_t renderObjects_length = array_length(renderObjects);
	for (size_t i = 0; i < renderObjects_length; i++) {
		// I could write `renderObjects + i`, but this hints to the reader that it's a pointer.
		renderObject_t *renderObject;
		int e = array_getElement(renderObjects, &renderObject, i);
		if (e) return e;
		renderRenderObject(renderObject);
	}
}

int render(entity_t *entity) {
	int e = ERR_CRITICAL;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// For each entity in the tree...
	
	// Set FOV.
	glUniform1f(g_screenHeightUniform, 16.0f/9.0f);

	e = allocator_create_stdlibArena(&g_renderObjectArena);
	(void) array_init(&g_transparencyRenderObjects, &g_renderObjectArena, sizeof(renderObject_t *));

	// Render world entity.
	e = renderEntity(entity, &(vec3_t){0, 0, 0}, &(quat_t){.s = 1, .v = {0, 0, 0}}, 1.0, -1);

	// Render transparent models.
	glEnable(GL_BLEND);
	renderRenderObjects(&g_transparencyRenderObjects);
	glDisable(GL_BLEND);

	// Destroy renderObjects and the array that contains them.
	e = g_renderObjectArena.quit(g_renderObjectArena.context);

	/* Show */
	
	SDL_GL_SwapWindow(g_window);
	
	return e;
}
