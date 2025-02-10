
#include "material.h"
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
// #define STBI_ASSERT(x)
#include "stb_image.h"
#include "../common/common.h"
#include "../common/log.h"
#include "../common/vfs.h"
#include "../common/memory.h"

material_list_t g_materialList;
size_t g_textures_length;


static void material_loadTexturemissingTexture(GLuint *textureIndex);
static int material_create(material_list_t *, ptrdiff_t *, GLuint, bool);
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
		// Default material is opaque.
		int e = material_create(&g_materialList, &materialIndex, textureIndex, false);
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
}

// static void material_free(material_t *material) {
	
// }

static int material_create(material_list_t *materialList, ptrdiff_t *materialIndex, GLuint textureIndex, bool transparent) {
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
	materialList->materials[lastIndex].transparent = transparent;
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

int l_material_create(lua_State *luaState) {
	int e = ERR_OK;

	GLuint textureIndex = 0;

	if (lua_gettop(luaState) != 1) {
		critical_error("Function requires 1 argument.", "");
		e = ERR_CRITICAL;
		goto cleanup;
	}

	if (!lua_isstring(luaState, -1)) {
		critical_error("Argument 1 should be a string, not a %s.", lua_typename(luaState, lua_type(luaState, -1)));
		e = ERR_CRITICAL;
		goto cleanup;
	}

	bool transparent = false;
	e = material_loadTexture(&textureIndex, &transparent, lua_tostring(luaState, -1));
	if (e) goto cleanup;

	ptrdiff_t materialIndex = -1;
	e = material_create(&g_materialList, &materialIndex, textureIndex, transparent);
	if (e) goto cleanup;

 cleanup:
	if (e >= ERR_CRITICAL) lua_error(luaState);

	if (e) {
		lua_pushinteger(luaState, -1);
	}
	else {
		lua_pushinteger(luaState, materialIndex);
	}

	lua_pushinteger(luaState, error);

	return 2;
}
