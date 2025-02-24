
#include <lauxlib.h>
#include "../common/types.h"
#include "../common/array.h"
#include "../common/str4.h"
#include "render.h"
#include "../common/common.h"
#include "../common/log.h"
#include "../common/vfs.h"
#include "../common/arena.h"
#include "../common/memory.h"

// The purpose of this array is so we can free all the shaders before we exit. That's all.
// Grows and never shrinks.
array_t g_shaders;

void shaders_init(void) {
	static Allocator a;
	a = allocator_create_stdlib();
	(void) array_init(&g_shaders, &a, sizeof(Shader *));
}

int shaders_quit(void) {
	size_t shaders_length = array_length(&g_shaders);
	for (size_t i = 0; i < shaders_length; i++) {
		Shader *shader;
		int e = array_getElement(&g_shaders, &shader, i);
		if (e) return e;
		MEMORY_FREE(&shader);
	}
	return array_quit(&g_shaders);
}

int shader_create(Shader **shader, Str4 *vertexShader_sourceCode, Str4 *fragmentShader_sourceCode) {
	int e = ERR_OK;
	do {
		Shader *localShader = malloc(sizeof(Shader));
		if (localShader == NULL) {
			outOfMemory();
			e = ERR_OUTOFMEMORY;
			break;
		}

		localShader->program = glCreateProgram();
		if (localShader->program == 0) {
			error("glCreateProgram returned with errors.", "");
			e = ERR_CRITICAL;
			break;
		}

		GLint compileStatus;

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		if (vertexShader == 0) {
			GLenum glError = glGetError();
			error("glCreateShader returned with errors.", "");
			while (glError) {
				error("OpenGL error: %s", render_glGetErrorString(glError));
				glError = glGetError();
			}
			e = ERR_CRITICAL;
			break;
		}
		glShaderSource(vertexShader, 1, (const char **) &(vertexShader_sourceCode->str), NULL);
		glCompileShader(vertexShader);
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus == GL_FALSE) {
			error("Vertex shader failed to compile.", "");
			render_logShaderInfo(vertexShader);
			e = ERR_CRITICAL;
			break;
		}
	
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		if (fragmentShader == 0) {
			GLenum glError = glGetError();
			error("glCreateShader returned with errors.", "");
			while (glError) {
				error("OpenGL error: %s", render_glGetErrorString(glError));
				glError = glGetError();
			}
			e = ERR_CRITICAL;
			break;
		}
		glShaderSource(fragmentShader, 1, (const char **) &(fragmentShader_sourceCode->str), NULL);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus == GL_FALSE) {
			error("Fragment shader failed to compile.", "");
			render_logShaderInfo(fragmentShader);
			e = ERR_CRITICAL;
			break;
		}
	
		glAttachShader(localShader->program, vertexShader);
		glAttachShader(localShader->program, fragmentShader);
		glLinkProgram(localShader->program);
		glGetProgramiv(localShader->program,  GL_LINK_STATUS, &compileStatus);
		if (compileStatus == GL_FALSE) {
			error("Program failed to link.", "");
			render_logProgramInfo(localShader->program);
			e = ERR_CRITICAL;
			break;
		}
		
		/* Find uniform variable locations. */
	
		localShader->uniform.orientation = glGetUniformLocation(localShader->program, "orientation");
		if (localShader->uniform.orientation < 0) {
			critical_error("Could not get uniform from program.", "");
			e = ERR_CRITICAL;
			break;
		}
	
		localShader->uniform.position = glGetUniformLocation(localShader->program, "position");
		if (localShader->uniform.position < 0) {
			critical_error("Could not get uniform from program.", "");
			e = ERR_CRITICAL;
			break;
		}

		localShader->uniform.scale = glGetUniformLocation(localShader->program, "scale");
		if (localShader->uniform.scale < 0) {
			critical_error("Could not get uniform from program.", "");
			e = ERR_CRITICAL;
			break;
		}
	
		localShader->uniform.aspectRatio = glGetUniformLocation(localShader->program, "aspectRatio");
		if (localShader->uniform.aspectRatio < 0) {
			critical_error("Could not get uniform from program.", "");
			e = ERR_CRITICAL;
			break;
		}

		localShader->instanced = false;

		e = array_push(&g_shaders, &localShader);
		if (e) break;

		*shader = localShader;
		printf("shaders_length %zu\n", g_shaders.elements_length);
		printf("shader %p\n", *shader);

	} while (0);
	return e;
}

int shader_load(Shader **shader, Str4 *shader_path) {
	int e = ERR_OK;

	Allocator a;
	e = allocator_create_stdlibArena(&a);
	if (e) goto cleanup;

	do {
		Str4 vertexShader_path = str4_create(&a);
		Str4 vertexShader_extension = STR4(".vert");
		(void) str4_concatenate(&vertexShader_path, shader_path, &vertexShader_extension);
		e = vertexShader_path.error;
		if (e) break;
		uint8_t *vertexShader_sourceCode_c;
		PHYSFS_sint64 vertexShader_sourceCode_c_length;
		e = vfs_getFileContents_malloc(&vertexShader_sourceCode_c, &vertexShader_sourceCode_c_length, vertexShader_path.str);
		if (e) break;
		Str4 vertexShader_sourceCode = str4_createConstant(vertexShader_sourceCode_c, vertexShader_sourceCode_c_length);

		Str4 fragmentShader_path = str4_create(&a);
		Str4 fragmentShader_extension = STR4(".frag");
		(void) str4_concatenate(&fragmentShader_path, shader_path, &fragmentShader_extension);
		e = fragmentShader_path.error;
		if (e) break;
		uint8_t *fragmentShader_sourceCode_c;
		PHYSFS_sint64 fragmentShader_sourceCode_c_length;
		e = vfs_getFileContents_malloc(&fragmentShader_sourceCode_c, &fragmentShader_sourceCode_c_length, fragmentShader_path.str);
		if (e) break;
		Str4 fragmentShader_sourceCode = str4_createConstant(fragmentShader_sourceCode_c, fragmentShader_sourceCode_c_length);

		e = shader_create(shader, &vertexShader_sourceCode, &fragmentShader_sourceCode);
		if (e) break;

	} while (0);
	int arena_e = a.quit(a.context);
	if (arena_e) e = arena_e;

 cleanup: return e;
}

// shader_create path
int l_shader_create(lua_State *l) {
	int e = ERR_OK;

	GLuint textureIndex = 0;

	if (lua_gettop(l) != 1) {
		critical_error("Function requires 1 argument.", "");
		e = ERR_CRITICAL;
		goto cleanup;
	}

	if (!lua_isstring(l, 1)) {
		critical_error("Argument 1 should be a string, not a %s.", lua_typename(l, lua_type(l, -1)));
		e = ERR_CRITICAL;
		goto cleanup;
	}

	size_t shader_path_c_length;
	const char *shader_path_c = lua_tolstring(l, 1, &shader_path_c_length);
	Str4 shader_path = str4_createConstant(shader_path_c, shader_path_c_length);

	Shader *shader;
	e = shader_load(&shader, &shader_path);
	if (e) goto cleanup;

	Shader **shaderReference = lua_newuserdata(l, sizeof(Shader *));
	*shaderReference = shader;
	// stack: shader

	(void) luaL_getmetatable(l, "shader");
	// stack: shader shader-type
	(void) lua_setmetatable(l, -2);
	// stack: shader

 cleanup:
	if (e >= ERR_CRITICAL) lua_error(l);
	(void) lua_pushinteger(l, e);
	return 2;
}

// shader_setInstanced shader value::(Boolean|Integer)
int l_shader_setInstanced(lua_State *l) {
	int e = ERR_OK;

	GLuint textureIndex = 0;

	if (lua_gettop(l) != 2) {
		critical_error("Function requires 2 arguments.", "");
		e = ERR_CRITICAL;
		goto cleanup;
	}

	Shader **shaderReference = luaL_checkudata(l, 1, "shader");
	if (shaderReference == NULL) {
		critical_error("Argument 1 should be a shader, not a %s.", lua_typename(l, lua_type(l, 1)));
		e = ERR_CRITICAL;
		goto cleanup;
	}
	Shader *shader = *shaderReference;

	if (!lua_isinteger(l, 2) && !lua_isboolean(l, 2)) {
		critical_error("Argument 2 should be an integer or a boolean, not a %s.", lua_typename(l, lua_type(l, 2)));
		e = ERR_CRITICAL;
		goto cleanup;
	}

	bool instanced;
	if (lua_isinteger(l, 2)) {
		instanced = 0 != lua_tointeger(l, 2);
	}
	if (lua_isboolean(l, 2)) {
		instanced = lua_toboolean(l, 2);
	}

	shader->instanced = instanced;

 cleanup:
	if (e >= ERR_CRITICAL) lua_error(l);
	lua_pushinteger(l, e);
	return 1;
}
