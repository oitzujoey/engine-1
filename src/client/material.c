
#include "material.h"
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
// #define STBI_ASSERT(x)
#include "stb_image.h"
#include "../common/common.h"
#include "../common/log.h"
#include "../common/vfs.h"

material_list_t g_materialList;
size_t g_textures_length;

void material_initList(material_list_t *materialList) {
	materialList->materials = NULL;
	materialList->materials_length = 0;
	g_textures_length = 0;
}

void material_freeList(material_list_t *materialList) {
	insane_free(materialList->materials);
	materialList->materials_length = 0;
}

static void material_init(material_t *material) {
	material->texture = 0;
}

// static void material_free(material_t *material) {
	
// }

static int material_create(material_list_t *materialList, ptrdiff_t *materialIndex) {
	
	materialList->materials_length++;
	materialList->materials = realloc(materialList->materials, materialList->materials_length * sizeof(material_t));
	if (materialList->materials == NULL) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	material_init(&materialList->materials[materialList->materials_length - 1]);
	
	cleanup_l:
	
	if (error) {
		*materialIndex = -1;
	}
	else {
		*materialIndex = materialList->materials_length - 1;
	}
	
	return error;
}

bool material_indexExists(const material_list_t materialList, const ptrdiff_t materialIndex) {
	return (materialIndex < materialList.materials_length) && (materialIndex >= 0);
}

static inline bool material_textureIndexExists(const GLuint textureIndex) {
	return (textureIndex < g_textures_length) && (textureIndex >= 0);
}

static inline void material_linkTexture(const material_list_t materialList, const ptrdiff_t materialIndex, GLuint textureIndex) {

	materialList.materials[materialIndex].texture = textureIndex;
}

static int material_loadTexture(GLuint *textureIndex, const char* const filePath) {
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
	}
	else if (channels == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
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
	
	insane_free(fileContents);
	fileContents_length = 0;
	
	stbi_image_free(pixels);
	
	cleanup_l:
	
	return error;
}


/* Lua bindings */
/* ============ */

int l_material_create(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	ptrdiff_t materialIndex = 0;
	
	if (lua_gettop(luaState) != 0) {
		critical_error("Function does not take arguments.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = material_create(&g_materialList, &materialIndex);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	if (error >= ERR_CRITICAL) {
		lua_error(luaState);
	}
	
	if (error) {
		lua_pushinteger(luaState, -1);
	}
	else {
		lua_pushinteger(luaState, materialIndex);
	}
	
	lua_pushinteger(luaState, error);
	
	return 2;
}

int l_material_linkTexture(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	ptrdiff_t materialIndex = 0;
	GLuint textureIndex = 0;
	
	if (lua_gettop(luaState) != 2) {
		critical_error("Function requires 2 arguments.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	if (!lua_isinteger(luaState, 1)) {
		critical_error("Argument 1 should be an integer, not a %s.", lua_typename(luaState, lua_type(luaState, -1)));
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	materialIndex = lua_tointeger(luaState, 1);
	if (!material_indexExists(g_materialList, materialIndex)) {
		error("Bad material index %i.", materialIndex);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (!lua_isinteger(luaState, 2)) {
		critical_error("Argument 2 should be an integer, not a %s.", lua_typename(luaState, lua_type(luaState, -1)));
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	textureIndex = lua_tointeger(luaState, 2);
	if (!material_textureIndexExists(materialIndex)) {
		error("Bad texture index %i.", textureIndex);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	material_linkTexture(g_materialList, materialIndex, textureIndex);
	
	error = ERR_OK;
	cleanup_l:
	
	if (error >= ERR_CRITICAL) {
		lua_error(luaState);
	}
	
	lua_pushinteger(luaState, error);
	
	return 1;
}

int l_material_loadTexture(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	GLuint textureIndex = 0;
	const char *filePath = NULL;
	
	if (lua_gettop(luaState) != 1) {
		critical_error("Function requires 1 argument.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	// if (!lua_isinteger(luaState, -2)) {
	// 	critical_error("Argument 2 should be an integer, not a %s.", lua_typename(luaState, lua_type(luaState, -2)));
	// 	error = ERR_CRITICAL;
	// 	goto cleanup_l;
	// }
	
	// textureIndex = lua_tointeger(luaState, -2);
	
	if (!lua_isstring(luaState, -1)) {
		critical_error("Argument 1 should be a string, not a %s.", lua_typename(luaState, lua_type(luaState, -1)));
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	filePath = lua_tostring(luaState, -1);
	
	error = material_loadTexture(&textureIndex, filePath);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	if (error >= ERR_CRITICAL) {
		lua_error(luaState);
	}
	
	if (error) {
		lua_pushinteger(luaState, -1);
	}
	else {
		lua_pushinteger(luaState, textureIndex);
	}
	
	lua_pushinteger(luaState, error);
	
	return 2;
}
