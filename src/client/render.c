
#include "render.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include "../common/common.h"
#include "../common/log.h"

SDL_Window* g_window;

GLuint g_programID = 0;
GLint g_vertexPosition2Dlocation = -1;
GLuint g_VBO = 0;
GLuint g_IBO = 0;
SDL_GLContext g_GLContext;

int render_initOpenGL(void) {
	int error = ERR_CRITICAL;
	
	GLenum glewError;
	GLuint vertexShader;
	GLuint fragmentShader;
	const GLchar *vertexShaderSource[] = {"#version 140\nin vec2 LVertexPos2D; void main() { gl_Position = vec4( LVertexPos2D.x, LVertexPos2D.y, 0, 1 ); }"};
	const GLchar *fragmentShaderSource[] = {"#version 140\nout vec4 LFragment; void main() { LFragment = vec4( 1.0, 1.0, 1.0, 1.0 ); }"};
	GLint shaderCompiled;
	GLint programSuccess;
	GLfloat vertexData[] = {
		-0.5, -0.5,
		 0.5, -0.5,
		 0.5,  0.5,
		-0.5,  0.5
	};
	GLuint indexData[] = {0, 1, 2, 3};
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	
	g_GLContext = SDL_GL_CreateContext(g_window);
	if (g_GLContext == NULL) {
		critical_error("Could not create OpenGL context. SDL error: %s", SDL_GetError());
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	glewExperimental = GL_TRUE;
	glewError = glewInit();
	if (glewError != GLEW_OK) {
		critical_error("Could not initialize GLEW. SDL error: %s", SDL_GetError());
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	g_programID = glCreateProgram();
	
	shaderCompiled = GL_FALSE;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != GL_TRUE) {
		critical_error("Failed to compile vertex shader %d.", vertexShader);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	glAttachShader(g_programID, vertexShader);
	
	shaderCompiled = GL_FALSE;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != GL_TRUE) {
		critical_error("Failed to compile fragment shader %d.", fragmentShader);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	glAttachShader(g_programID, fragmentShader);
	
	glLinkProgram(g_programID);
	programSuccess = GL_TRUE;
	glGetProgramiv(g_programID, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE) {
		critical_error("Error linking program %d.", g_programID);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	g_vertexPosition2Dlocation = glGetAttribLocation(g_programID, "LVertexPos2D");
	if (g_vertexPosition2Dlocation == -1) {
		critical_error("\"LVertexPos2D\" is not a GLSL program variable.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	glClearColor(0.0, 0.0, 0.0, 1.0);
	
	glGenBuffers(1, &g_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);
	
	glGenBuffers(1, &g_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLint), indexData, GL_STATIC_DRAW);
	
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(g_programID);
	
	glEnableVertexAttribArray(g_vertexPosition2Dlocation);
	
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glVertexAttribPointer(g_vertexPosition2Dlocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);
	
	glDisableVertexAttribArray(g_vertexPosition2Dlocation);
	
	glUseProgram(0);
	
	SDL_GL_SwapWindow(g_window);
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int render(lua_State *L) {

	// SDL_FillRect(g_screenSurface, NULL, SDL_MapRGB(g_screenSurface->format, 0xFF, 0xFF, 0xFF));
	
	// SDL_UpdateWindowSurface(g_window);
	
	return 0;
}
