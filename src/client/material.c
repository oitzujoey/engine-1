
#include "material.h"
#include <GL/glew.h>
#include <lauxlib.h>
#define STB_IMAGE_IMPLEMENTATION
// #define STBI_ASSERT(x)
#include "stb_image.h"
#include "../common/common.h"
#include "../common/log.h"
#include "../common/vfs.h"
#include "../common/memory.h"
#include "shader.h"

material_list_t g_materialList;
size_t g_textures_length;


static void material_loadTexturemissingTexture(GLuint *textureIndex);
static int material_create(material_list_t *, ptrdiff_t *, Shader *, GLuint, bool);
static void material_linkTexture(const material_list_t, const ptrdiff_t, GLuint);


int material_initList(material_list_t *materialList) {
	materialList->materials = NULL;
	materialList->materials_length = 0;
	g_textures_length = 0;

	{
		// Load hot pink texture as the default texture so that we know when a texture is missing on a model.
		GLuint textureIndex;
		ptrdiff_t materialIndex;
		(void) material_loadTexturemissingTexture(&textureIndex);

		// Create a default shader. The projection matrix almost certainly won't look right when used with shaders
		// loaded from files, but it should be better than nothing.
		Shader *shader = NULL;
		{
			// Would be cool if I could load this from a file at compile time.
			Str4 vertexShader_sourceCode = STR4("#version 400\n"
			                                    "layout(location = 0) in vec3 vp;"
			                                    "layout(location = 1) in vec3 normal;"
			                                    "layout(location = 2) in vec2 texCoord;"
			                                    "uniform vec4 orientation;"
			                                    "uniform vec3 position;"
			                                    "uniform float aspectRatio;"
			                                    "uniform float scale;"
			                                    "out vec3 color;"
			                                    "out vec2 textureCoordinate;"
			                                    "vec4 conjugate(vec4 a) {"
			                                    "  return vec4(-a.x,"
			                                    "              -a.y,"
			                                    "              -a.z,"
			                                    "              a.w);"
			                                    "}"
			                                    "vec4 hamilton(vec4 a, vec4 b) {"
			                                    "  return vec4(a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,"
			                                    "              a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,"
			                                    "              a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,"
			                                    "              a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z);"
			                                    "}"
			                                    "vec3 rotate(vec3 v, vec4 q) {"
			                                    "  vec4 v4 = vec4(v, 0);"
			                                    "  v4 = hamilton(hamilton(q, v4), conjugate(q));"
			                                    "  return vec3(v4.x, v4.y, v4.z);"
			                                    "}"
			                                    "float w = 11.0*aspectRatio;"
			                                    "float h = 11.0;"
			                                    "float n = 11.0;"
			                                    "float f = 7500.0;"
			                                    "mat4 projectionMatrix = mat4(-2.0*n/w, 0.0, 0.0, 0.0,"
			                                    "                             0.0, 2.0*n/h, 0.0, 0.0,"
			                                    "                             0.0, 0.0, -(f+n)/(f-n), -1.0,"
			                                    "                             0.0, 0.0, -2.0*f*n/(f-n), 0.0);"
			                                    "void main() {"
			                                    "  vec3 vertex;"
			                                    "  vertex = rotate(scale * vp, orientation);"
			                                    "  vertex += position;"
			                                    "  gl_Position = projectionMatrix * vec4(vertex, 1.0);"
			                                    "  color = rotate(normal, orientation);"
			                                    "  textureCoordinate = texCoord;"
			                                    "}");
			Str4 fragmentShader_sourceCode = STR4("#version 400\n"
			                                      "out vec4 frag_colour;"
			                                      "in vec3 color;"
			                                      "in vec2 textureCoordinate;"
			                                      "uniform sampler2D ourTexture;"
			                                      "void main() {"
			                                      "  float dot = dot(color, vec3(0.0, 0.0, 1.0));"
			                                      "  float mixing = 0.75;"
			                                      "  dot = abs(dot) * (1.0 - mixing) + mixing;"
			                                      "  frag_colour = texture(ourTexture, textureCoordinate)"
			                                      "                        * vec4(dot, dot, dot, 1.0);"
			                                      "}");
			int e = shader_create(&shader, &vertexShader_sourceCode, &fragmentShader_sourceCode);
		}

		// Default material is opaque.
		int e = material_create(&g_materialList, &materialIndex, shader, textureIndex, false);
		if (e) return e;
	}

	return ERR_OK;
}

void material_freeList(material_list_t *materialList) {
	MEMORY_FREE(&materialList->materials);
	materialList->materials_length = 0;
}

static void material_init(material_t *material) {
	material->texture = 0;
	material->transparent = false;
	material->depthSort = false;
	material->cull = true;
}

// static void material_free(material_t *material) {
	
// }

static int material_create(material_list_t *materialList,
                           ptrdiff_t *materialIndex,
                           Shader *shader,
                           GLuint textureIndex,
                           bool transparent) {
	int e = ERR_OK;

	materialList->materials_length++;
	materialList->materials = realloc(materialList->materials, materialList->materials_length * sizeof(material_t));
	if (materialList->materials == NULL) {
		outOfMemory();
		e = ERR_OUTOFMEMORY;
		goto cleanup;
	}

	size_t lastIndex = materialList->materials_length - 1;
	(void) material_init(&materialList->materials[lastIndex]);
	materialList->materials[lastIndex].shader = shader;
	materialList->materials[lastIndex].transparent = transparent;
	materialList->materials[lastIndex].depthSort = transparent;  // sic.
	(void) material_linkTexture(*materialList, lastIndex, textureIndex);

 cleanup:
	*materialIndex = e ? -1 : lastIndex;
	return e;
}

bool material_indexExists(const material_list_t materialList, const ptrdiff_t materialIndex) {
	return (materialIndex < materialList.materials_length) && (materialIndex >= 0);
}

static bool material_textureIndexExists(const GLuint textureIndex) {
	return (textureIndex < g_textures_length) && (textureIndex >= 0);
}

static void material_linkTexture(const material_list_t materialList, const ptrdiff_t materialIndex, GLuint textureIndex) {
	materialList.materials[materialIndex].texture = textureIndex;
}

// This should be the first texture loaded, so the texture index will be 1.
static void material_loadTexturemissingTexture(GLuint *textureIndex) {
	int width = 1;
	int height = 1;
	int channels = 3;
	// Hot Pink.
	uint8_t pixels[3] = {0xff, 0x45, 0xfc};

	// Dummy index. Nobody sees this. We just assume it will be 0.
	glGenTextures(1, textureIndex);
	glBindTexture(GL_TEXTURE_2D, *textureIndex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	g_textures_length++;
}

static int material_loadTexture(GLuint *textureIndex, bool *transparent, const char* const filePath) {
	int error = ERR_CRITICAL;
	
	// Load image file.
	stbi_uc *fileContents;
	PHYSFS_sint64 fileContents_length;
	int width;
	int height;
	int channels;
	
	if (!PHYSFS_exists(filePath)) {
		error("File \"%s\" does not exist", filePath);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = vfs_getFileContents_malloc(&fileContents, &fileContents_length, filePath);
	if (error) {
		error("Could not read text from file \"%s\".", filePath);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Decode image.
	stbi_uc *pixels = stbi_load_from_memory(fileContents, fileContents_length, &width, &height, &channels, 0);
	if (pixels == NULL) {
		critical_error("Could not convert image data to pixels.", "");
		error = ERR_CRITICAL;
		goto cleanupPHYSFS_l;
	}
	
	// printf("%llu\n%s\n", fileContents_length, fileContents);
	// network_dumpBufferUint8(fileContents, fileContents_length);
	
	// Load image into OpenGL.
	
	*textureIndex = 0;
	glGenTextures(1, textureIndex);
	glBindTexture(GL_TEXTURE_2D, *textureIndex);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (channels == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		// Opaque.
		*transparent = false;
	}
	else if (channels == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		// Transparent, meaning this texture has an alpha channel.
		*transparent = true;
	}
	else {
		error("Unsupported number of channels: %i", channels);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	// glGenerateMipmap(GL_TEXTURE_2D);
	
	g_textures_length++;
	
	error = ERR_OK;
	cleanupPHYSFS_l:
	
	MEMORY_FREE(&fileContents);
	fileContents_length = 0;
	
	stbi_image_free(pixels);
	
	cleanup_l:
	
	return error;
}


/* Lua bindings */
/* ============ */

// material_create shaderPath texturePath
// `shaderPath` does not have the file extension because this function adds the .vert and .frag suffixes.
int l_material_create(lua_State *luaState) {
	int e = ERR_OK;

	GLuint textureIndex = 0;

	if (lua_gettop(luaState) != 2) {
		critical_error("Function requires 2 arguments.", "");
		e = ERR_CRITICAL;
		goto cleanup;
	}

	Shader **shaderReference = luaL_checkudata(luaState, 1, "shader");
	if (shaderReference == NULL) {
		critical_error("Argument 1 should be a shader, not a %s.", lua_typename(luaState, lua_type(luaState, 1)));
		e = ERR_CRITICAL;
		goto cleanup;
	}
	Shader *shader = *shaderReference;

	if (!lua_isstring(luaState, 2)) {
		critical_error("Argument 2 should be a string, not a %s.", lua_typename(luaState, lua_type(luaState, 2)));
		e = ERR_CRITICAL;
		goto cleanup;
	}

	bool transparent = false;
	e = material_loadTexture(&textureIndex, &transparent, lua_tostring(luaState, 2));
	if (e) goto cleanup;

	ptrdiff_t materialIndex = -1;
	e = material_create(&g_materialList, &materialIndex, shader, textureIndex, transparent);
	if (e) goto cleanup;

 cleanup:
	if (e >= ERR_CRITICAL) lua_error(luaState);

	if (e) {
		lua_pushinteger(luaState, -1);
	}
	else {
		lua_pushinteger(luaState, materialIndex);
	}

	(void) lua_pushinteger(luaState, e);

	return 2;
}

int l_material_setDepthSort(lua_State *l) {
	int e = ERR_OK;

	GLuint textureIndex = 0;

	if (lua_gettop(l) != 2) {
		critical_error("Function requires 2 arguments.", "");
		e = ERR_CRITICAL;
		goto cleanup;
	}

	if (!lua_isinteger(l, 1)) {
		critical_error("Argument 1 should be the material index, an integer, not a %s.", lua_typename(l, lua_type(l, 1)));
		e = ERR_CRITICAL;
		goto cleanup;
	}

	if (!lua_isinteger(l, 2) && !lua_isboolean(l, 2)) {
		critical_error("Argument 2 should be an integer or a boolean, not a %s.", lua_typename(l, lua_type(l, 2)));
		e = ERR_CRITICAL;
		goto cleanup;
	}

	lua_Integer materialIndex = lua_tointeger(l, 1);
	if (!material_indexExists(g_materialList, materialIndex)) {
		error("Bad material index %i.", materialIndex);
		e = ERR_GENERIC;
		goto cleanup;
	}
	material_t *material = &g_materialList.materials[materialIndex];

	bool doDepthSort;
	if (lua_isinteger(l, 2)) {
		doDepthSort = 0 != lua_tointeger(l, 2);
	}
	if (lua_isboolean(l, 2)) {
		doDepthSort = lua_toboolean(l, 2);
	}

	material->depthSort = doDepthSort;

 cleanup:
	if (e >= ERR_CRITICAL) lua_error(l);
	lua_pushinteger(l, e);
	return 1;
}

int l_material_setCull(lua_State *l) {
	int e = ERR_OK;

	GLuint textureIndex = 0;

	if (lua_gettop(l) != 2) {
		critical_error("Function requires 2 arguments.", "");
		e = ERR_CRITICAL;
		goto cleanup;
	}

	if (!lua_isinteger(l, 1)) {
		critical_error("Argument 1 should be the material index, an integer, not a %s.", lua_typename(l, lua_type(l, 1)));
		e = ERR_CRITICAL;
		goto cleanup;
	}

	if (!lua_isinteger(l, 2) && !lua_isboolean(l, 2)) {
		critical_error("Argument 2 should be an integer or a boolean, not a %s.", lua_typename(l, lua_type(l, 2)));
		e = ERR_CRITICAL;
		goto cleanup;
	}

	lua_Integer materialIndex = lua_tointeger(l, 1);
	if (!material_indexExists(g_materialList, materialIndex)) {
		error("Bad material index %i.", materialIndex);
		e = ERR_GENERIC;
		goto cleanup;
	}
	material_t *material = &g_materialList.materials[materialIndex];

	bool cull;
	if (lua_isinteger(l, 2)) {
		cull = 0 != lua_tointeger(l, 2);
	}
	if (lua_isboolean(l, 2)) {
		cull = lua_toboolean(l, 2);
	}

	material->cull = cull;

 cleanup:
	if (e >= ERR_CRITICAL) lua_error(l);
	lua_pushinteger(l, e);
	return 1;
}
