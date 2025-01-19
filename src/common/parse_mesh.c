#include "types.h"
#include "common.h"
#include "file.h"
#include "vfs.h"
#include "log.h"
#include "str2.h"
#include "obj.h"
#include "vector.h"
#include "str3.h"
#include "memory.h"
#include <stdio.h>


static int cmsh_parse(file_cmsh_t *cmsh, uint8_t *bytes, size_t length) {
	int e = ERR_OK;

	size_t index = 0;

	for (size_t i = 0; i < sizeof(cmsh->magic)/sizeof(uint8_t); i++) {
		e = file_parse_uint8(&cmsh->magic[i], bytes, &index, length);
		if (e) return e;
	}
	if (!str3_eq(STR3("CMSH"), cmsh->magic, sizeof(cmsh->magic))) {
		error("Magic number is not CMSH.", "");
		return ERR_GENERIC;
	}
	e = file_parse_uint32(&cmsh->version, bytes, &index, length);
	if (e) return e;
	e = file_parse_vec3(cmsh->mins, bytes, &index, length);
	if (e) return e;
	e = file_parse_vec3(cmsh->maxs, bytes, &index, length);
	if (e) return e;
	e = file_parse_uint32(&cmsh->vertices_length, bytes, &index, length);
	if (e) return e;
	cmsh->vertices = malloc(cmsh->vertices_length * sizeof(vec_t));
	if (!cmsh->vertices) return ERR_OUTOFMEMORY;
	e = file_parse_vecArray(cmsh->vertices, cmsh->vertices_length, bytes, &index, length);
	if (e) {
		memory_free(cmsh->vertices);
		cmsh->vertices_length = 0;
	}
	return e;
}

static int rmsh_parse(file_rmsh_t *rmsh, uint8_t *bytes, size_t length) {
	int e = ERR_OK;

	size_t index = 0;

	for (size_t i = 0; i < sizeof(rmsh->magic)/sizeof(uint8_t); i++) {
		e = file_parse_uint8(&rmsh->magic[i], bytes, &index, length);
		if (e) return e;
	}
	if (!str3_eq(STR3("RMSH"), rmsh->magic, sizeof(rmsh->magic))) {
		error("Magic number is not RMSH.", "");
		return ERR_GENERIC;
	}
	e = file_parse_uint32(&rmsh->version, bytes, &index, length);
	if (e) return e;
	e = file_parse_vec3(rmsh->mins, bytes, &index, length);
	if (e) return e;
	e = file_parse_vec3(rmsh->maxs, bytes, &index, length);
	if (e) return e;
	e = file_parse_uint32(&rmsh->vertices_length, bytes, &index, length);
	if (e) return e;
	e = file_parse_uint32(&rmsh->vertexNormals_length, bytes, &index, length);
	if (e) return e;
	e = file_parse_uint32(&rmsh->vertexTextureCoords_length, bytes, &index, length);
	if (e) return e;
	do {
		rmsh->vertices = malloc(rmsh->vertices_length * sizeof(vec_t));
		rmsh->vertexNormals = malloc(rmsh->vertexNormals_length * sizeof(vec_t));
		rmsh->vertexTextureCoords = malloc(rmsh->vertexTextureCoords_length * sizeof(vec_t));
		if (!rmsh->vertices || !rmsh->vertexNormals || !rmsh->vertexTextureCoords) {
			e = ERR_OUTOFMEMORY;
			break;
		}
		e = file_parse_vecArray(rmsh->vertices, rmsh->vertices_length, bytes, &index, length);
		if (e) break;
		e = file_parse_vecArray(rmsh->vertexNormals, rmsh->vertexNormals_length, bytes, &index, length);
		if (e) break;
		e = file_parse_vecArray(rmsh->vertexTextureCoords, rmsh->vertexTextureCoords_length, bytes, &index, length);
		if (e) break;
	} while (0);
	if (e) {
		if (!rmsh->vertices) memory_free(rmsh->vertices);
		if (!rmsh->vertexNormals) memory_free(rmsh->vertexNormals);
		if (!rmsh->vertexTextureCoords) memory_free(rmsh->vertexTextureCoords);
		rmsh->vertices_length = 0;
		rmsh->vertexNormals_length = 0;
		rmsh->vertexTextureCoords_length = 0;
	}
	return e;
}


// `fileName` is null terminated.
// `cmsh->vertices` must be freed.
static int cmsh_read(file_cmsh_t *cmsh, uint8_t *fileName) {
	uint8_t *bytes = NULL;
	PHYSFS_sint64 bytes_length;

	int e = vfs_getFileContents_malloc(&bytes, &bytes_length, fileName);
	if (e) return e;
	return cmsh_parse(cmsh, bytes, bytes_length);
}

// `fileName` is null terminated.
// `rmsh->vertices` must be freed.
static int rmsh_read(file_rmsh_t *rmsh, uint8_t *fileName) {
	uint8_t *bytes = NULL;
	PHYSFS_sint64 bytes_length;

	int e = vfs_getFileContents_malloc(&bytes, &bytes_length, fileName);
	if (e) return e;
	return rmsh_parse(rmsh, bytes, bytes_length);
}


void cmsh_print(file_cmsh_t *cmsh) {
	puts("cmsh {");
	char *m = (char *) cmsh->magic;
	printf("  magic: %c%c%c%c\n", m[0], m[1], m[2], m[3]);
	printf("  version: %u\n", cmsh->version);
	printf("  mins: {%f, %f, %f}\n", cmsh->mins[0], cmsh->mins[1], cmsh->mins[2]);
	printf("  maxs: {%f, %f, %f}\n", cmsh->maxs[0], cmsh->maxs[1], cmsh->maxs[2]);
	printf("  vertices[%u] {\n", cmsh->vertices_length);
	for (size_t i = 0; i < cmsh->vertices_length; i += 3) {
		printf("    %f, %f, %f,\n", cmsh->vertices[i], cmsh->vertices[i + 1], cmsh->vertices[i + 2]);
	}
	puts("  }");
	puts("}");
}

void rmsh_print(file_rmsh_t *rmsh) {
	puts("rmsh {");
	char *m = (char *) rmsh->magic;
	printf("  magic: %c%c%c%c\n", m[0], m[1], m[2], m[3]);
	printf("  version: %u\n", rmsh->version);
	printf("  mins: {%f, %f, %f}\n", rmsh->mins[0], rmsh->mins[1], rmsh->mins[2]);
	printf("  maxs: {%f, %f, %f}\n", rmsh->maxs[0], rmsh->maxs[1], rmsh->maxs[2]);
	printf("  vertices[%u] {\n", rmsh->vertices_length);
	for (size_t i = 0; i < rmsh->vertices_length; i += 3) {
		printf("    %f, %f, %f,\n", rmsh->vertices[i], rmsh->vertices[i + 1], rmsh->vertices[i + 2]);
	}
	puts("  }");
	printf("  vertexNormals[%u] {\n", rmsh->vertexNormals_length);
	for (size_t i = 0; i < rmsh->vertexNormals_length; i += 3) {
		printf("    %f, %f, %f,\n", rmsh->vertexNormals[i], rmsh->vertexNormals[i + 1], rmsh->vertexNormals[i + 2]);
	}
	puts("  }");
	printf("  vertexTextureCoords[%u] {\n", rmsh->vertexTextureCoords_length);
	for (size_t i = 0; i < rmsh->vertexTextureCoords_length; i += 2) {
		printf("    %f, %f,\n",
		       rmsh->vertexTextureCoords[i],
		       rmsh->vertexTextureCoords[i + 1]);
	}
	puts("  }");
	puts("}");
}


// `filePath` is null terminated.
static int cmsh_load(size_t *index, uint8_t *filePath) {
	int e = ERR_OK;

	model_t *model = NULL;
	file_cmsh_t cmsh = {0};

	e = modelList_createModel(&model, index);
	if (e) {
		error("Failed to create new model", "");
		return e;
	}

	e = cmsh_read(&cmsh, filePath);
	if (e) goto cleanup;

	(void) cmsh_print(&cmsh);

	// `model` now has ownership of cmsh's vertices buffer.
	model->collision_vertices = cmsh.vertices;
	model->collision_vertices_length = cmsh.vertices_length;
	(void) vec3_copy(&model->bb_mins, &cmsh.mins);
	(void) vec3_copy(&model->bb_maxs, &cmsh.maxs);
	// The AABB will change in the future, but since we start off with no rotation, BB == AABB.
	(void) vec3_copy(&model->aabb_mins, &cmsh.mins);
	(void) vec3_copy(&model->aabb_maxs, &cmsh.maxs);

 cleanup:
	if (e) {
		if (cmsh.vertices) memory_free(cmsh.vertices);
		// Can't free models?!? I guess that's OK?
	}
	return e;
}

// `filePath` is null terminated.
static int rmsh_load(size_t *index, uint8_t *filePath) {
	int e = ERR_OK;

	model_t *model = NULL;
	file_rmsh_t rmsh = {0};

	e = modelList_createModel(&model, index);
	if (e) {
		error("Failed to create new model", "");
		return e;
	}

	e = rmsh_read(&rmsh, filePath);
	if (e) goto cleanup;

	(void) rmsh_print(&rmsh);

	(void) vec3_copy(&model->bb_mins, &rmsh.mins);
	(void) vec3_copy(&model->bb_maxs, &rmsh.maxs);
	// The AABB will change in the future, but since we start off with no rotation, BB == AABB.
	(void) vec3_copy(&model->aabb_mins, &rmsh.mins);
	(void) vec3_copy(&model->aabb_maxs, &rmsh.maxs);

#ifdef CLIENT
	// `model` now has ownership of rmsh's vertices buffer.
	model->glVertices = rmsh.vertices;
	model->glVertices_length = rmsh.vertices_length;
	// `model` now has ownership of rmsh's vertex normals buffer.
	model->glNormals = rmsh.vertexNormals;
	model->glNormals_length = rmsh.vertexNormals_length;
	// `model` now has ownership of rmsh's texture coordinates buffer.
	model->glTexCoords = rmsh.vertexTextureCoords;
	model->glTexCoords_length = rmsh.vertexTextureCoords_length;

	// Only allow one texture to be bound to the model.
	model->numBindableMaterials = 1;
	model->defaultMaterials = malloc(model->numBindableMaterials * sizeof(ptrdiff_t));
	if (model->defaultMaterials == NULL) {
		e = ERR_OUTOFMEMORY;
		goto cleanup;
	}
#endif

 cleanup:
	if (e) {
		if (rmsh.vertices) memory_free(rmsh.vertices);
		if (rmsh.vertexNormals) memory_free(rmsh.vertexNormals);
		if (rmsh.vertexTextureCoords) memory_free(rmsh.vertexTextureCoords);
#ifdef CLIENT
		if (model->defaultMaterials) memory_free(model->defaultMaterials);
#endif
		// Can't free models?!? I guess that's OK?
	}
	return e;
}


int l_cmsh_load(lua_State *luaState) {
	int e = 0;

	uint8_t *filePath = NULL;
	size_t index;

	if (!lua_isstring(luaState, 1)) {
		error("Argument 1 must be a string.", "");
		e = ERR_GENERIC;
		goto cleanup;
	}
	e = str3_copyMalloc(&filePath, lua_tostring(luaState, 1));
	if (e) goto cleanup;

	e = cmsh_load(&index, filePath);
	if (e) goto cleanup;

 cleanup:
	memory_free(filePath);

	if (e == ERR_OUTOFMEMORY) {
		outOfMemory();
		lua_error(luaState);
	}

	lua_pushinteger(luaState, index);
	lua_pushinteger(luaState, e);

	return 2;
}

int l_rmsh_load(lua_State *luaState) {
	int e = 0;

	uint8_t *filePath = NULL;
	size_t index;

	if (!lua_isstring(luaState, 1)) {
		error("Argument 1 must be a string.", "");
		e = ERR_GENERIC;
		goto cleanup;
	}
	e = str3_copyMalloc(&filePath, lua_tostring(luaState, 1));
	if (e) goto cleanup;

	e = rmsh_load(&index, filePath);
	if (e) goto cleanup;

 cleanup:
	memory_free(filePath);

	if (e == ERR_OUTOFMEMORY) {
		outOfMemory();
		lua_error(luaState);
	}

	lua_pushinteger(luaState, index);
	lua_pushinteger(luaState, e);

	return 2;
}
