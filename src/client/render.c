
#include "render.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
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
GLuint g_positionVbo;
GLuint g_orientationVbo;
GLuint g_scaleVbo;
GLuint g_vao;
char *g_openglLogFileName;
float g_points[] = {
	-0.5, -0.5, 0,
	0.5, -0.5, 0,
	0, 0.5, 0
};
Allocator g_renderObjectArena;

array_t g_transparencyRenderObjects;
array_t g_sortedTransparencyRenderObjects;
array_t g_instancedRenderObjects;


const char *render_glGetErrorString(GLenum glError) {
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

	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

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

	g_positionVbo = 0;
	glGenBuffers(1, &g_positionVbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_positionVbo);

	g_orientationVbo = 0;
	glGenBuffers(1, &g_orientationVbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_orientationVbo);

	g_scaleVbo = 0;
	glGenBuffers(1, &g_scaleVbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_scaleVbo);

	
	g_vao = 0;
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, g_VertexVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glBindBuffer(GL_ARRAY_BUFFER, g_texCoordVbo);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, g_positionVbo);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, g_orientationVbo);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, g_scaleVbo);
	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

	/* Set background color. */
	
	glClearColor(0.1, 0.1, 0.1, 1.0);
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

// Note: `position` is `vec3_t *` and not `vec3_t` because `vec3_t` is decayed to a pointer. I'm not entirely clear on
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
		Shader *shader = material->shader;

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

		bool transparent = material->transparent;
		bool instanced = shader->instanced;
		// I believe that (material, shader) is guaranteed to be in the list of material-shader pairs if it is instanced.
		if (transparent && instanced) {
			// Perhaps we should name objects so that we can refer to them in error messages?
			warning("Scene graph node is both transparent and GPU instanced. This is an invalid configuration.", "");
		} else if (transparent) {
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

			if (material->depthSort) {
				return array_push(&g_sortedTransparencyRenderObjects, &ro);
			}
			else {
				return array_push(&g_transparencyRenderObjects, &ro);
			}
		} else if (instanced) {
			// Insert `iro` into the array selected by its shader-model pair.
			size_t model_key = modelIndex;
			size_t shader_key = shader->shader_index;
			GLuint texture_key = material->texture;
			// Search for shader-model pair.
			bool missing = true;
			InstancedRenderObjects *iro = NULL;
			for (size_t index = 0; index < g_instancedRenderObjects.elements_length; index++) {
				e = array_getElement(&g_instancedRenderObjects, &iro, index);
				if (e) return e;
				if (iro->shader_index == shader_key && iro->model_index == model_key && iro->texture_index == texture_key) {
					missing = false;
					break;
				}
			}
			if (missing) {
				// Shader-model pair doesn't exist, so create a new one.
				// Create an instance object for each shader, even if the shader isn't used.
				e = ARENA_ALLOC(g_renderObjectArena, &iro, sizeof(InstancedRenderObjects));
				if (e) return e;
				iro->shader_index = shader_key;
				iro->model_index = model_key;
				iro->texture_index = texture_key;
				iro->glVertices_length = model->glVertices_length;
				iro->glVertices = model->glVertices;
				iro->glNormals_length = model->glNormals_length;
				iro->glNormals = model->glNormals;
				iro->glTexCoords_length = model->glTexCoords_length;
				iro->glTexCoords = model->glTexCoords;
				iro->material = material;
				(void) array_init(&iro->orientations, &g_renderObjectArena, sizeof(quat_t));
				(void) array_init(&iro->positions, &g_renderObjectArena, sizeof(vec3_t));
				(void) array_init(&iro->scales, &g_renderObjectArena, sizeof(vec_t));
			}

			float glOrientation[4] = {orientation.v[0], orientation.v[1], orientation.v[2], orientation.s};
			e = array_push(&iro->orientations, &glOrientation);
			if (e) return e;
			e = array_push(&iro->positions, position);
			if (e) return e;
			e = array_push(&iro->scales, &scale);
			if (e) return e;

			if (missing) {
				// Put the shader-model pair into the associative array.
				e = array_push(&g_instancedRenderObjects, &iro);
				if (e) return e;
			}
			return e;
		}

		/* Render */

		glUseProgram(shader->program);

		glBindBuffer(GL_ARRAY_BUFFER, g_VertexVbo);
		glBufferData(GL_ARRAY_BUFFER, model->glVertices_length * sizeof(float), model->glVertices, GL_DYNAMIC_DRAW);
		
		glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
		glBufferData(GL_ARRAY_BUFFER, model->glNormals_length * sizeof(float), model->glNormals, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, g_texCoordVbo);
		glBufferData(GL_ARRAY_BUFFER, model->glTexCoords_length * sizeof(float), model->glTexCoords, GL_DYNAMIC_DRAW);

		glUniform1f(shader->uniform.aspectRatio, 16.0f/9.0f);
		glUniform4f(shader->uniform.orientation,
			orientation.v[0],
			orientation.v[1],
			orientation.v[2],
			orientation.s
		);
		glUniform3f(shader->uniform.position,
		            (*position)[0],
		            (*position)[1],
		            (*position)[2]);
		glUniform1f(shader->uniform.scale, scale);

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

int renderInstanced(array_t *instancedRenderObjects) {
	array_t *iros = instancedRenderObjects;
	Allocator *a = iros->allocator;
	size_t iros_length = array_length(iros);
	printf("draws: %zu\n", iros_length);
	for (size_t iros_index = 0; iros_index < iros_length; iros_index++) {
		InstancedRenderObjects *iro;
		int e = array_getElement(iros, &iro, iros_index);
		if (e) return e;

		/* For normal renderObjects, we would iterate over the objects and render them as we extract them. In this case,
		   we only need one InstancedRenderObjects to extract the model and shader information from, then we pass the
		   position, orientation, and scale to the GPU. */
		size_t iro_length = array_length(&iro->positions);
		{
			material_t *material = iro->material;
			Shader *shader = material->shader;

			glUseProgram(shader->program);

			glBindBuffer(GL_ARRAY_BUFFER, g_VertexVbo);
			glBufferData(GL_ARRAY_BUFFER, iro->glVertices_length * sizeof(float), iro->glVertices, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
			glBufferData(GL_ARRAY_BUFFER, iro->glNormals_length * sizeof(float), iro->glNormals, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, g_texCoordVbo);
			glBufferData(GL_ARRAY_BUFFER, iro->glTexCoords_length * sizeof(float), iro->glTexCoords, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, g_positionVbo);
			glBufferData(GL_ARRAY_BUFFER, iro_length * 3 * sizeof(float), iro->positions.elements, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glVertexAttribDivisor(3, 1);

			glBindBuffer(GL_ARRAY_BUFFER, g_orientationVbo);
			glBufferData(GL_ARRAY_BUFFER, iro_length * 4 * sizeof(float), iro->orientations.elements, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glVertexAttribDivisor(4, 1);

			glBindBuffer(GL_ARRAY_BUFFER, g_scaleVbo);
			glBufferData(GL_ARRAY_BUFFER, iro_length * sizeof(float), iro->scales.elements, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glVertexAttribDivisor(5, 1);

			glUniform1f(shader->uniform.aspectRatio, 16.0f/9.0f);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, iro->texture_index);
			glBindVertexArray(g_vao);

			if (material->cull) glEnable(GL_CULL_FACE);

			// glVertices_length is always a multiple of three.
			glDrawArraysInstanced(GL_TRIANGLES, 0, iro->glVertices_length / 3, iro_length);
			glDisable(GL_CULL_FACE);
		}
	}
	return 0;
}

void renderRenderObject(renderObject_t *renderObject) {
	material_t *material = renderObject->material;
	Shader *shader = material->shader;

	glUseProgram(shader->program);

	glBindBuffer(GL_ARRAY_BUFFER, g_VertexVbo);
	glBufferData(GL_ARRAY_BUFFER, renderObject->glVertices_length * sizeof(float), renderObject->glVertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
	glBufferData(GL_ARRAY_BUFFER, renderObject->glNormals_length * sizeof(float), renderObject->glNormals, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, g_texCoordVbo);
	glBufferData(GL_ARRAY_BUFFER, renderObject->glTexCoords_length * sizeof(float), renderObject->glTexCoords, GL_DYNAMIC_DRAW);

	glUniform1f(shader->uniform.aspectRatio, 16.0f/9.0f);
	glUniform4f(shader->uniform.orientation,
				renderObject->orientation.v[0],
				renderObject->orientation.v[1],
				renderObject->orientation.v[2],
				renderObject->orientation.s
				);
	glUniform3f(shader->uniform.position,
				renderObject->position[0],
				renderObject->position[1],
				renderObject->position[2]);
	glUniform1f(shader->uniform.scale, renderObject->scale);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, material->texture);
	glBindVertexArray(g_vao);

	if (material->cull) glEnable(GL_CULL_FACE);

	// glVertices_length is always a multiple of three.
	glDrawArrays(GL_TRIANGLES, 0, renderObject->glVertices_length / 3);
	glDisable(GL_CULL_FACE);
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
	return ERR_OK;
}

int renderObject_compare(const void *a, const void *b) {
	const renderObject_t *roa = *((const renderObject_t **) a);
	const renderObject_t *rob = *((const renderObject_t **) b);
	// The array is traversed front to back when rendering. We want farther objects to be drawn first, so farther
	// objects should appear first in the array. So a larger magnitude is counted as smaller.
	const vec_t result = vec3_norm2(&rob->position) - vec3_norm2(&roa->position);
	// Cast to integer.
	return (result > 0
	        ? 1
	        : result < 0
	        ? -1
	        : 0);
}

int render(entity_t *entity) {
	int e = ERR_CRITICAL;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	e = allocator_create_stdlibArena(&g_renderObjectArena);
	(void) array_init(&g_transparencyRenderObjects, &g_renderObjectArena, sizeof(renderObject_t *));
	(void) array_init(&g_sortedTransparencyRenderObjects, &g_renderObjectArena, sizeof(renderObject_t *));
	(void) array_init(&g_instancedRenderObjects, &g_renderObjectArena, sizeof(InstancedRenderObjects *));


	// Render world node.
	glEnable(GL_CULL_FACE);
	e = renderEntity(entity, &(vec3_t){0, 0, 0}, &(quat_t){.s = 1, .v = {0, 0, 0}}, 1.0, -1);
	glDisable(GL_CULL_FACE);


	// Render scene nodes with instanced shaders.
	e = renderInstanced(&g_instancedRenderObjects);
	if (e) goto cleanup;


	// Sort transparent models. I think this is expensive.
	(void) qsort(g_sortedTransparencyRenderObjects.elements,
	             g_sortedTransparencyRenderObjects.elements_length,
	             g_sortedTransparencyRenderObjects.element_size,
	             renderObject_compare);

	// Render unsorted transparent models.
	glEnable(GL_BLEND);
	e = renderRenderObjects(&g_transparencyRenderObjects);
	if (e) goto cleanup;

	// Render sorted transparent models on top of unsorted transparent models.
	e = renderRenderObjects(&g_sortedTransparencyRenderObjects);
	if (e) goto cleanup;
	glDisable(GL_BLEND);


	/* Show */
	SDL_GL_SwapWindow(g_window);

 cleanup:
	// Destroy renderObjects and the array that contains them.
	e = g_renderObjectArena.quit(g_renderObjectArena.context);
	return e;
}
