
#include "render.h"
#include <stdio.h>
#include <stdarg.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include "../common/common.h"
#include "../common/log.h"
#include "../common/entity.h"
#include "../common/obj.h"
#include "../common/vector.h"

SDL_Window* g_window;
SDL_DisplayMode g_displayMode;
SDL_GLContext g_GLContext;
GLuint g_VertexVbo;
GLuint g_colorVbo;
GLuint g_vao;
GLuint g_shaderProgram[2];
char *g_openglLogFileName;
float g_points[] = {
	-0.5, -0.5, 0,
	0.5, -0.5, 0,
	0, 0.5, 0
};


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

int render_handle_updateLogFileName(cfg_var_t *var) {
	g_openglLogFileName = var->string.value;
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
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);
	// glFrontFace(GL_CW);
	
	g_VertexVbo = 0;
	glGenBuffers(1, &g_VertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_VertexVbo);
	// glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), g_points, GL_STATIC_DRAW);
	// glBufferData(GL_ARRAY_BUFFER, 27 * sizeof(float), g_points, GL_DYNAMIC_DRAW);
	// glError = glGetError();
	// if (glError) {
	// 	error("glBufferData returned with errors.", "");
	// 	while (glError) {
	// 		error("OpenGL error: %s", render_glGetErrorString(glError));
	// 		glError = glGetError();
	// 	}
	// 	error = ERR_CRITICAL;
	// 	goto cleanup_l;
	// }
	
	g_colorVbo = 0;
	glGenBuffers(1, &g_colorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
	// glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), g_points, GL_STATIC_DRAW);
	// glBufferData(GL_ARRAY_BUFFER, 27 * sizeof(float), g_points, GL_DYNAMIC_DRAW);
	// glError = glGetError();
	// if (glError) {
	// 	error("glBufferData returned with errors.", "");
	// 	while (glError) {
	// 		error("OpenGL error: %s", render_glGetErrorString(glError));
	// 		glError = glGetError();
	// 	}
	// 	error = ERR_CRITICAL;
	// 	goto cleanup_l;
	// }
	
	g_vao = 0;
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);
	glBindBuffer(GL_ARRAY_BUFFER, g_VertexVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	const char* vertexShaderSource0 =
		"#version 400\n"
		"layout(location = 0) in vec3 vp;"
		"layout(location = 1) in vec3 vc;"
		"out vec3 color;"
		"void main() {"
		"  vec3 vertex;"
		"  vertex = vp;"
		"  gl_Position = vec4(vertex, 1.0);"
		"  color = vc;"
		"}";
	
	const char* fragmentShaderSource0 =
		"#version 400\n"
		"out vec4 frag_colour;"
		"in vec3 color;"
		"void main() {"
		// "  float b = 2 * gl_FragCoord.z;"
		// "  frag_colour = vec4(b, b, b, 1.0);"
		"  float dot = dot(color, vec3(0.0, 0.0, 1.0));"
		"  frag_colour = vec4(abs(dot), abs(dot), abs(dot), 1.0);"
		// "  frag_colour = vec4(length(color), length(color), length(color), 1.0);"
		// "  frag_colour = vec4(color.x, color.y, color.z, 1.0);"
		"}";
	
	const char* fragmentShaderSource1 =
		"#version 400\n"
		"out vec4 frag_colour;"
		"in vec3 color;"
		"void main() {"
		"  frag_colour = vec4(0.0, 0.0, 0.0, 1.0);"
		"}";
	
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
	
	GLuint fragmentShader1 = glCreateShader(GL_FRAGMENT_SHADER);
	if (fragmentShader1 == 0) {
		glError = glGetError();
		error("glCreateShader returned with errors.", "");
		while (glError) {
			error("OpenGL error: %s", render_glGetErrorString(glError));
			glError = glGetError();
		}
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	glShaderSource(fragmentShader1, 1, &fragmentShaderSource1, NULL);
	glCompileShader(fragmentShader1);
	glGetShaderiv(fragmentShader1, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		error("Fragment shader failed to compile.", "");
		render_logShaderInfo(fragmentShader1);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	glAttachShader(g_shaderProgram[1], vertexShader0);
	glAttachShader(g_shaderProgram[1], fragmentShader1);
	glLinkProgram(g_shaderProgram[1]);
	glGetProgramiv(g_shaderProgram[1],  GL_LINK_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		error("Program failed to link.", "");
		render_logProgramInfo(g_shaderProgram[1]);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	glClearColor(0.1, 0.1, 0.1, 1.0);
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int render(lua_State *L) {

	// SDL_FillRect(g_screenSurface, NULL, SDL_MapRGB(g_screenSurface->format, 0xFF, 0xFF, 0xFF));
	
	// SDL_UpdateWindowSurface(g_window);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// For each entity...
	for (int i = 0; i < g_entityList.entities_length; i++) {
		if (!g_entityList.entities[i].inUse) {
			// Don't draw deleted entities.
			continue;
		}
		
		if (g_entityList.entities[i].childType != entity_childType_model) {
			// Literally nothing to see here.
			continue;
		}
		
		// For each model...
		for (int j = 0; j < g_entityList.entities[i].children_length; j++) {
			
			int modelIndex = g_entityList.entities[i].children[j];
			model_t model = g_modelList.models[modelIndex];
			GLsizeiptr facesLength = model.faces_length;
			float points[9 * facesLength];
			float normals[9 * facesLength];
			
			const vec_t scale = 0.015f;
			
			// For each face...
			for (int k = 0; k < facesLength; k++) {
				// For each face vertex...
				for (int l = 0; l < 3; l++) {
					vec3_t point;
					vec3_t normal;
					const vec3_t screen = {
						(vec_t) g_displayMode.h / (vec_t) g_displayMode.w,
						1.0,
						1.0
					};
					
					vec3_copy(&normal, &model.surface_normals[k]);
					vec3_rotate(&normal, &g_entityList.entities[i].orientation);
					
					vec3_copy(&point, &model.vertices[model.faces[k][l]]);
					vec3_rotate(&point, &g_entityList.entities[i].orientation);
					
					// For each vertex axis...
					for (int m = 0; m < 3; m++) {
						points[9 * k + 3 * l + m] = point[m] * screen[m] * scale;
						
						// Each normal will be duplicated at least three times.
						normals[9 * k + 3 * l + m] = normal[m];
					}
				}
			}
			
			// For each point...
			for (int k = 0; k < 3 * facesLength; k++) {
				for (int l = 0; l < 3; l++) {
					points[3 * k + l] += g_entityList.entities[i].position[l] * scale;
				}
			}
			
			glBindBuffer(GL_ARRAY_BUFFER, g_VertexVbo);
			glBufferData(GL_ARRAY_BUFFER, 9 * facesLength * sizeof(float), points, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
			glBufferData(GL_ARRAY_BUFFER, 9 * facesLength * sizeof(float), normals, GL_DYNAMIC_DRAW);
			glUseProgram(g_shaderProgram[0]);
			glBindVertexArray(g_vao);
			glDrawArrays(GL_TRIANGLES, 0, 3 * facesLength);
			// glUseProgram(g_shaderProgram[1]);
			// glBindVertexArray(g_vao);
			// glDrawArrays(GL_LINE_STRIP, 0, 3 * facesLength);
		}
	}
	
	SDL_GL_SwapWindow(g_window);
	
	return 0;
}
