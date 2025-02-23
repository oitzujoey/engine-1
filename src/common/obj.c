
#include "obj.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include "log.h"
#include "vfs.h"
#include "cfg2.h"
#include "common.h"
#include "file.h"
#include "vector.h"
#include "str2.h"
#include "memory.h"
#ifdef CLIENT
#include "../client/material.h"
#endif

/* I should probably rewrite this. */

/* Model list */
/* ========== */

modelList_t g_modelList;

/* obj_isValidModelIndex
index:i     The index of a model.
Returns:    1 if index is valid, otherwise 0.
*/
int obj_isValidModelIndex(size_t index) {
	
	// Out of bounds.
	if (index < 0) {
		return 0;
	}
	if (index >= g_modelList.models_length) {
		return 0;
	}

	return 1;
}


/* modelList_createModel
model:o         The address of the model.
index:o         The index of the model.
Returns:        error
Globals:        g_modelList
Description:    Creates a model and returns the address and index in the list.
*/
int modelList_createModel(model_t **model, size_t *index) {
	int error = ERR_OK;

	if (g_modelList.models_length < g_modelList.models_length_actual) {
		g_modelList.models_length++;
	}
	else {
		g_modelList.models_length++;
		g_modelList.models_length_actual++;
		g_modelList.models = realloc(g_modelList.models, g_modelList.models_length_actual * sizeof(model_t));
		if (g_modelList.models == NULL) {
			outOfMemory();
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
	}
	*model = &g_modelList.models[g_modelList.models_length - 1];
	*index = g_modelList.models_length - 1;
	
	model_init(*model);
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

/* modelList_removeLastModel
Globals:        g_modelList
Description:    Ignores the last model that was created.
*/
void modelList_removeLastModel(void) {
	--g_modelList.models_length;
}

/* modelList_free
Globals:        g_modelList
Description:    Frees all models in the list and then frees the list iself.
*/
void modelList_free(void) {
	for (int i = 0; i < g_modelList.models_length_actual; i++) {
		model_free(&g_modelList.models[i]);
	}
	MEMORY_FREE(&g_modelList.models);
	g_modelList.models_length = 0;
	g_modelList.models_length_actual = 0;
}

/* modelList_init
Globals:        g_modelList
Description:    Initialize the model list.
*/
void modelList_init(void) {
	g_modelList.models = NULL;
	g_modelList.models_length = 0;
	g_modelList.models_length_actual = 0;
}

/* Model */
/* ===== */

void model_init(model_t *model) {
	model->faces = NULL;
	model->faces_length = 0;
	model->surface_normals = NULL;
	model->vertices = NULL;
	model->vertices_length = 0;
#ifdef CLIENT
	model->instanced = false;
	model->glVertices = NULL;
	model->glNormals = NULL;
	model->boundingSphere = 0.0;
	model->defaultMaterials = NULL;
	model->defaultMaterials_index = 0;
	model->texCoords = NULL;
	model->texCoords_textures = NULL;
	model->numBindableMaterials = 0;
	model->glTexCoords = NULL;
#endif
}

void model_free(model_t *model) {
	MEMORY_FREE(&model->vertices);
	model->vertices_length = 0;
	MEMORY_FREE(&model->surface_normals);
	for (int i = 0; i < model->faces_length; i++) {
		MEMORY_FREE(&model->faces[i]);
	}
	MEMORY_FREE(&model->faces);
#ifdef CLIENT
	MEMORY_FREE(&model->glVertices);
	MEMORY_FREE(&model->glNormals);
	model->boundingSphere = 0;
	MEMORY_FREE(&model->defaultMaterials);
	model->defaultMaterials_index = 0;
	for (ptrdiff_t i = 0; i < model->faces_length; i++) {
		MEMORY_FREE(&model->texCoords[i]);
	}
	MEMORY_FREE(&model->texCoords);
	MEMORY_FREE(&model->texCoords_textures);
	MEMORY_FREE(&model->glTexCoords);
#endif
	model->faces_length = 0;
}

#ifdef CLIENT

static inline int model_linkDefaultMaterial(ptrdiff_t parentIndex, ptrdiff_t materialIndex) {
	int error = ERR_CRITICAL;

	if (g_modelList.models[parentIndex].defaultMaterials_index >= g_modelList.models[parentIndex].numBindableMaterials) {
		warning("Model list full. Aborting operation.", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	g_modelList.models[parentIndex].defaultMaterials[g_modelList.models[parentIndex].defaultMaterials_index++] = materialIndex;
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int l_model_linkDefaultMaterial(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	ptrdiff_t materialIndex = -1;
	ptrdiff_t modelIndex = -1;
	
	if (lua_gettop(luaState) != 2) {
		critical_error("Function requires 2 arguments.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	if (!lua_isinteger(luaState, 1)) {
		critical_error("Argument 1 should be an integer, not a %s.", lua_typename(luaState, lua_type(luaState, 1)));
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	if (!lua_isinteger(luaState, 2)) {
		critical_error("Argument 2 should be an integer, not a %s.", lua_typename(luaState, lua_type(luaState, 2)));
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	modelIndex = lua_tointeger(luaState, 1);
	if (!obj_isValidModelIndex(modelIndex)) {
		error("Bad entity index %i.", modelIndex);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	materialIndex = lua_tointeger(luaState, 2);
	if (!material_indexExists(g_materialList, materialIndex)) {
		error("Bad material index %i.", materialIndex);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = model_linkDefaultMaterial(modelIndex, materialIndex);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	if (error >= ERR_CRITICAL) {
		lua_error(luaState);
	}
	
	lua_pushinteger(luaState, error);
	
	return 1;
}

int l_model_setInstanced(lua_State *l) {
	int e = ERR_OK;

	GLuint textureIndex = 0;

	if (lua_gettop(l) != 2) {
		critical_error("Function requires 2 arguments.", "");
		e = ERR_CRITICAL;
		goto cleanup;
	}

	if (!lua_isinteger(l, 1)) {
		critical_error("Argument 1 should be the model index, an integer, not a %s.", lua_typename(l, lua_type(l, -1)));
		e = ERR_CRITICAL;
		goto cleanup;
	}

	if (!lua_isinteger(l, 2) && !lua_isboolean(l, 2)) {
		critical_error("Argument 2 should be an integer or a boolean, not a %s.", lua_typename(l, lua_type(l, -1)));
		e = ERR_CRITICAL;
		goto cleanup;
	}

	lua_Integer modelIndex = lua_tointeger(l, 1);
	if (!obj_isValidModelIndex(modelIndex)) {
		error("Bad model index %i.", modelIndex);
		e = ERR_GENERIC;
		goto cleanup;
	}
	model_t *model = &g_modelList.models[modelIndex];

	bool instanced;
	if (lua_isinteger(l, 2)) {
		instanced = 0 != lua_tointeger(l, 2);
	}
	if (lua_isboolean(l, 2)) {
		instanced = lua_toboolean(l, 2);
	}

	model->instanced = instanced;

 cleanup:
	if (e >= ERR_CRITICAL) lua_error(l);
	lua_pushinteger(l, e);
	return 1;
}

#endif


/* Oolite DAT */
/* ========== */

/*
We will need a dedicated list of models. I think only one type will be required
since each type of model should be able to be converted to a unified format. We
will call that format "model_t".
*/

/* obj_loadOoliteDAT
filePath:i      Workspace path to file.
index:o         Index of the model in the model list.
Returns:        error
Globals:        g_modelList
Description:    Load an Oolite model from a .dat file and add it into the model
	list. "index" is set to the index of the model in the model list.
*/
int obj_loadOoliteDAT(const char *filePath, size_t *index) {
	int error = ERR_OK;

	char *fileText = NULL;
	size_t fileText_length;
	char *line = NULL;
	size_t line_length = 0;
	char *tempPointer0 = NULL;
	char *tempPointer1 = NULL;
	int datProgress = 0;
	char *newlinePointer = NULL;
#ifdef CLIENT
	vec_t tempVec = 0.0f;
#endif
	
	enum {
		mode_start,
		mode_nverts,
		mode_nfaces,
		mode_vertex,
		mode_faces,
		mode_textures,
		mode_names,
		mode_normals,
		mode_end
	} mode = mode_start;
	
	int argc;
	const int maxArgs = 10;
	char *argv[maxArgs];
	model_t *model;
	int tempModelElementIndex;
	vec3_t tempVec3[2];
	const int progressMask
	    = (1<<mode_start)   | (1<<mode_nverts)  | (1<<mode_nfaces)
	    | (1<<mode_vertex)  | (1<<mode_faces)   | (1<<mode_end)
	    | (1<<mode_textures);
	
	// model_init(model);
	// Create a model.
	modelList_createModel(&model, index);
	
	datProgress |= 1<<mode_start;
	
	for (int i = 0; i < maxArgs; i++) {
		argv[i] = NULL;
	}

	/* Get text from file. */

	error = vfs_getFileText(&fileText, filePath);
	if (error) {
		error("vfs_getFileText returned ", ERR[error]);
		error = error;
		goto cleanup_l;
	}
	
	fileText_length = strlen(fileText);
	
	/* Convert all \r to \n. */
	for (int i = 0; i < fileText_length; i++) {
		if (fileText[i] == '\r') {
			fileText[i] = '\n';
		}
	}
	
	/* Go through each line in file. */
	
	for (int lineNumber = 1;; lineNumber++) {
		
		/* Get the line. */
		
		if (lineNumber == 1) {
			line = fileText;
		}
		else {
			line = newlinePointer + 1;
			if (line > fileText + fileText_length) {
				break;
			}
		}
		
		newlinePointer = strchr(line, '\n');
		if (newlinePointer == NULL) {
			newlinePointer = fileText + fileText_length;
		}
		
		line_length = newlinePointer - line;
		line[line_length] = '\0';
		
		/* Remove comments and unnecessary whitespace. */
		
		str2_removeLineComments(line, "//");
		str2_removeWhitespace(line, "melt");
		// Convert remaining whitespace to spaces.
		for (int i = 0; i < line_length; i++) {
			if (isspace(line[i])) {
				line[i] = ' ';
			}
		}

		// Remove empty lines.
		if (!strcmp(line, "")) {
			continue;
		}
		
		/* Split line into arguments. */
		
		tempPointer0 = line;
		argc = 1;
		while (*tempPointer0 != '\0') {
			if (*tempPointer0++ == ' ') {
				argc++;
			}
		}
		
		if (argc > maxArgs) {
			error("Line %i of file \"%s\" has too many arguments. %i > maxArgs(%i)", lineNumber, filePath, argc, maxArgs);
			error = ERR_GENERIC;
			goto cleanup_l;
		}
		
		for (int i = 0; i < argc; i++) {
			
			if (i == 0)  {
				tempPointer0 = line;
			}
			tempPointer1 = strchr(tempPointer0, ' ');
			if (tempPointer1 == NULL) {
				tempPointer1 = line + line_length;
			}
			
			argv[i] = tempPointer0;
			argv[i][tempPointer1 - tempPointer0] = '\0';
			
			tempPointer0 = tempPointer1 + 1;
		}
		
		/* Execute command if one exists. */
		
		if (!strcmp(argv[0], "NVERTS")) {
			
			if (argc != 2) {
				error("\"%s\":%i \"%s\" requires 2 arguments", filePath, lineNumber, argv[0]);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			model->vertices_length = strtol(argv[1], NULL, 10);
			// I should really be checking for INT_MAX and INT_MIN.
			
			if (model->vertices_length != 0) {
				model->vertices = malloc(model->vertices_length * sizeof(vec3_t));
			}
			else {
				model->vertices = NULL;
			}
			
			// Does nothing special.
			mode = mode_nverts;
			datProgress |= 1<<mode_nverts;
			continue;
		}
		if (!strcmp(argv[0], "NFACES")) {
			
			if (argc != 2) {
				error("\"%s\":%i \"%s\" requires 2 arguments", filePath, lineNumber, argv[0]);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			model->faces_length = strtol(argv[1], NULL, 10);
			// I should really be checking for INT_MAX and INT_MIN.
			
			// Allocate memory now that we know the number of faces there are.
			if (model->faces_length != 0) {
				model->faces = malloc(model->faces_length * sizeof(int *));
				if (model->faces == NULL) {
					outOfMemory();
					error = ERR_OUTOFMEMORY;
					goto cleanup_l;
				}
				for (int i = 0; i < model->faces_length; i++) {
					model->faces[i] = malloc(3 * sizeof(int));
					if (model->faces[i] == NULL) {
						outOfMemory();
						error = ERR_OUTOFMEMORY;
						goto cleanup_l;
					}
				}
				
#				ifdef CLIENT
				model->texCoords = malloc(model->faces_length * sizeof(vec2_t *));
				if (model->texCoords == NULL) {
					outOfMemory();
					error = ERR_OUTOFMEMORY;
					goto cleanup_l;
				}
				for (ptrdiff_t i = 0; i < model->faces_length; i++) {
					model->texCoords[i] = malloc(3 * sizeof(vec2_t));
					if (model->texCoords[i] == NULL) {
						outOfMemory();
						error = ERR_OUTOFMEMORY;
						goto cleanup_l;
					}
				}
				
				model->texCoords_textures = malloc(model->faces_length * sizeof(vec2_t *));
				if (model->texCoords_textures == NULL) {
					outOfMemory();
					error = ERR_OUTOFMEMORY;
					goto cleanup_l;
				}
#				endif
			}
			else {
				model->faces = NULL;
#				ifdef CLIENT
				model->texCoords = NULL;
				model->texCoords_textures = NULL;
#				endif
			}
			
			// Does nothing special.
			mode = mode_nfaces;
			datProgress |= 1<<mode_nfaces;
			continue;
		}
		if (!strcmp(argv[0], "VERTEX")) {
			
			if (argc != 1) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath, lineNumber, argv[0]);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			if (((1<<mode_nverts) & datProgress) == 0) {
				error("\"%s\":%i NVERTS has not been declared", filePath, lineNumber);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			tempModelElementIndex = 0;
			
			mode = mode_vertex;
			// Don't set progress until the last vertex has been set.
			// datProgress |= 1<<mode_vertex;
			continue;
		}
		if (!strcmp(argv[0], "FACES")) {
			
			if (argc != 1) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath, lineNumber, argv[0]);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			if (((1<<mode_nfaces) & datProgress) == 0) {
				error("\"%s\":%i NFACES has not been declared", filePath, lineNumber);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			tempModelElementIndex = 0;
			
			mode = mode_faces;
			// Don't set progress until the last face has been set.
			// datProgress |= 1<<mode_faces;
			continue;
		}
		if (!strcmp(argv[0], "TEXTURES")) {
			
			if (argc != 1) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath, lineNumber, argv[0]);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			if (((1<<mode_nfaces) & datProgress) == 0) {
				error("\"%s\":%i NFACES has not been declared", filePath, lineNumber);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			tempModelElementIndex = 0;
			
			mode = mode_textures;
			continue;
		}
		if (!strcmp(argv[0], "NAMES")) {
			
			if (argc != 2) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath, lineNumber, argv[0]);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			// @TODO: Do something about the names.
			
			tempModelElementIndex = 0;
			
			mode = mode_names;
			continue;
		}
		if (!strcmp(argv[0], "NORMALS")) {
			
			if (argc != 1) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath, lineNumber, argv[0]);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			// @TODO: Do something about the normals.
			
			tempModelElementIndex = 0;
			
			mode = mode_normals;
			continue;
		}
		if (!strcmp(argv[0], "END")) {
			
			// Picky, picky.
			if (argc != 1) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath, lineNumber, argv[0]);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			datProgress |= 1<<mode_end;
			mode = mode_end;
			break;
		}
		
		/* Do the appropriate action for the current mode. */
		switch (mode) {
			case mode_start:
				continue;
			case mode_nverts:
				continue;
			case mode_nfaces:
				continue;
			
			case mode_vertex:
				
				if (argc != 3) {
					error("\"%s\":%i mode \"vertex\" requires 3 arguments", filePath, lineNumber);
					error = ERR_GENERIC;
					goto cleanup_l;
				}
				
				if (tempModelElementIndex >= model->vertices_length) {
					error("\"%s\":%i Vertex list is longer than declared by NVERTS(%i)", filePath, lineNumber, model->vertices_length);
					error = ERR_GENERIC;
					goto cleanup_l;
				}
				
				for (int i = 0; i < 3; i++) {
					model->vertices[tempModelElementIndex][i] = strtof(argv[i], NULL);
				}
#ifdef CLIENT
				tempVec = sqrt(
					model->vertices[tempModelElementIndex][0]*model->vertices[tempModelElementIndex][0] +
					model->vertices[tempModelElementIndex][1]*model->vertices[tempModelElementIndex][1] +
					model->vertices[tempModelElementIndex][2]*model->vertices[tempModelElementIndex][2]
				);
				if (model->boundingSphere < tempVec) {
					model->boundingSphere = tempVec;
				}
#endif
				tempModelElementIndex++;
				
				if (tempModelElementIndex == model->vertices_length) {
					// Done.
					datProgress |= 1<<mode_vertex;
				}
				
				continue;
			
			case mode_faces:
				
				if (argc != 10) {
					error("\"%s\":%i mode \"faces\" requires 10 arguments", filePath, lineNumber);
					error = ERR_GENERIC;
					goto cleanup_l;
				}
				
				if (tempModelElementIndex >= model->faces_length) {
					error("\"%s\":%i Face list is longer than declared by NFACES(%i)", filePath, lineNumber, model->faces_length);
					error = ERR_GENERIC;
					goto cleanup_l;
				}
				
				for (int i = 0; i < 3; i++) {
					model->faces[tempModelElementIndex][i] = strtof(argv[i + 7], NULL);
				}
				tempModelElementIndex++;
				
				if (tempModelElementIndex == model->faces_length) {
					// Done.
					datProgress |= 1<<mode_faces;
				}
				
				continue;
			
			case mode_textures:
				
				if (argc != 9) {
					error("\"%s\":%i mode \"faces\" requires 9 arguments", filePath, lineNumber);
					error = ERR_GENERIC;
					goto cleanup_l;
				}
				
				if (tempModelElementIndex >= model->faces_length) {
					error("\"%s\":%i Texture list is longer than declared by NFACES(%i)", filePath, lineNumber, model->faces_length);
					error = ERR_GENERIC;
					goto cleanup_l;
				}
				
				// for (int i = 0; i < 3; i++) {
				// 	model->faces[tempModelElementIndex][i] = strtof(argv[i + 7], NULL);
				// }
#				ifdef CLIENT
				model->texCoords_textures[tempModelElementIndex] = strtol(argv[0], NULL, 10);
				// Bad coding. Do not attempt at home.
				if (model->texCoords_textures[tempModelElementIndex] > model->numBindableMaterials) {
					model->numBindableMaterials = model->texCoords_textures[tempModelElementIndex];
				}
				
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 2; j++) {
						model->texCoords[tempModelElementIndex][i][j] = strtof(argv[2*i + j + 3], NULL);
					}
				}
#				endif
				tempModelElementIndex++;
				
				if (tempModelElementIndex == model->faces_length) {
					// Done.
					datProgress |= 1<<mode_textures;
#					ifdef CLIENT
					model->numBindableMaterials++;
					
					model->defaultMaterials = malloc(model->numBindableMaterials * sizeof(ptrdiff_t));
					if (model->defaultMaterials == NULL) {
						outOfMemory();
						error = ERR_OUTOFMEMORY;
						goto cleanup_l;
					}
#					endif
				}
				
				continue;
			
			case mode_names:
				continue;
			case mode_normals:
				continue;
			case mode_end:
				error("Can't happen. \"mode\" is set to \"mode_end\".", "");
				break;
			default:
				error("Can't happen. \"mode\" is set to the invalid value %i.", mode);
				continue;
		}
	}

	// Now that we are finished loading the model, let's see what we actually accomplished.
	if (datProgress != progressMask) {
		error("\"%s\" Model not fully defined by file", filePath);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Create suface normals.
	model->surface_normals = malloc(model->faces_length * sizeof(vec3_t));
	if (model->surface_normals == NULL) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	/* I think this is correct. Subtract vertex 0 from 1 & 2 to create two
	   vectors. The cross product will be normal to both. */
	for (int i = 0; i < model->faces_length; i++) {
		// Create vectors from the three face points. Two vectors define a plane.
		vec3_subtract(&tempVec3[0], &model->vertices[model->faces[i][1]], &model->vertices[model->faces[i][0]]);
		vec3_subtract(&tempVec3[1], &model->vertices[model->faces[i][2]], &model->vertices[model->faces[i][0]]);
		// Create normal from vectors.
		vec3_crossProduct(&model->surface_normals[i], &tempVec3[0], &tempVec3[1]);
		// And finally, normalize the normal.
		error = vec3_normalize(&model->surface_normals[i]);
		if (error) {
			error("Cannot create normal due to invalid face %i", "");
			error = ERR_GENERIC;
			goto cleanup_l;
		}
	}
	
#ifdef CLIENT
	// Load vertices and normals into arrays now to save time when rendering.
	model->glVertices = malloc(3 * 3 * model->faces_length * sizeof(vec_t));
	if (model->glVertices == NULL) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	model->glNormals = malloc(3 * 3 * model->faces_length * sizeof(vec_t));
	if (model->glNormals == NULL) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	model->glTexCoords = malloc(2 * 3 * model->faces_length * sizeof(vec_t));
	if (model->glTexCoords == NULL) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	// For each face...
	for (int k = 0; k < model->faces_length; k++) {
		// For each face vertex...
		for (int l = 0; l < 3; l++) {
			// For each vertex axis...
			for (int m = 0; m < 3; m++) {
				model->glVertices[3 * 3 * k + 3 * l + m] = model->vertices[model->faces[k][l]][m];// * screen[m];
				
				// Each normal will be duplicated at least three times. :(
				// 2024-01-13: THANK YOU. Now I don't have to merge three .obj vertex normals into a single face normal.
				model->glNormals[3 * 3 * k + 3 * l + m] = model->surface_normals[k][m];
			}
			
			for (ptrdiff_t m = 0; m < 2; m++) {
				model->glTexCoords[2 * 3 * k + 2 * l + m] = model->texCoords[k][l][m];
			}
		}
	}
	model->glVertices_length = model->faces_length * 3 * 3;
	model->glNormals_length = model->faces_length * 3 * 3;
	model->glTexCoords_length = model->faces_length * 3 * 2;
#endif
	
	error = ERR_OK;
	cleanup_l:
	
	if (error) {
		warning("\"%s\" Discarding model due to earlier errors.", filePath);
		model_free(model);
		modelList_removeLastModel();
	}
	
	// for (int i = 0; i < maxArgs; i++) {
	// 	string_free(&argv[i]);
	// }
	// string_free(&line);
	MEMORY_FREE(&fileText);
	
	return error;
}

int l_obj_loadOoliteDAT(lua_State *luaState) {
	int error = 0;
	
	size_t index;
	char *filePath = NULL;
	
	if (!lua_isstring(luaState, 1)) {
		error("Argument 1 must be a string.", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	error = str2_copyMalloc(&filePath, lua_tostring(luaState, 1));
	if (error) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	error = obj_loadOoliteDAT(filePath, &index);
	if (error) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	error = 0;
	cleanup_l:
	
	MEMORY_FREE(&filePath);
	
	if (error == ERR_OUTOFMEMORY) {
		outOfMemory();
		lua_error(luaState);
	}
	if (error) {
		critical_error("Uncaught error.", "");
		lua_error(luaState);
	}
	
	lua_pushinteger(luaState, index);
	lua_pushinteger(luaState, error);
	
	return 2;
}

/* MTL */
/* === */

// static int mtl_parsestring(mtl_t *mtl, const char *string) {
// }

// static int mtl_print(mtl_t *mtl) {
// }

// static int mtl_init(mtl_t *mtl) {
// }

// static int mtl_free(mtl_t *mtl) {
// }

// int obj_loadMTL(obj_t *obj) {
	
// 	string_t fileText;
// 	string_t filePath;
// 	int localError = 0;
// 	/* This is not really a string. It should never be init or freed. */
// 	string_t line;
// 	int tempIndex;
// 	bool acceptCommands = false;
// 	string_t command;
// 	int argc;
// 	const int maxArgs = 3;
// 	string_t argv[maxArgs];
	
// 	string_init(&fileText);
// 	string_init(&filePath);
// 	string_init(&line);
// 	string_init(&command);
// 	for (int i = 0; i < maxArgs; i++) {
// 		string_init(&argv[i]);
// 	}
	
// 	if ((obj->material_name.length == 0) || (obj->material_library.length == 0)) {
// 		return ERR_GENERIC;
// 	}
	
// 	string_copy(&filePath, &obj->material_library);
	
// 	log_info(__func__, "Loading material \"%s\"", filePath.value);
	
// 	localError = vfs_getFileText(&g_vfs, &fileText, &filePath);
// 	if (localError) {
// 		log_error(__func__, "Could not open file \"%s\"", filePath.value);
// 		goto cleanup_l;
// 	}
	
// 	for (int lineNumber = 1;; lineNumber++) {
		
// 		tempIndex = string_index_of(&fileText, lineNumber - 1, '\n');
// 		if (tempIndex == -1) {
// 			break;
// 		}
// 		tempIndex++;
			
// 		string_substring(&line, &fileText, tempIndex, string_index_of(&fileText, lineNumber, '\n') - tempIndex);
		
// 		/* Remove comments and unnecessary whitespace. */
// 		string_removeLineComments(&line, "#");
// 		string_removeWhitespace(&line, "melt");
// 		/* Convert remaining whitespace to spaces */
// 		for (int i = 0; i < line.length; i++) {
// 			if (isspace(line.value[i])) {
// 				line.value[i] = ' ';
// 			}
// 		}

// 		/* Ignore empty lines */
// 		if (line.length == 0) {
// 			continue;
// 		}

// 		argc = string_count_char(&line, ' ');
// 		if (argc > maxArgs) {
// 			log_critical_error(__func__, "Please reprimand the developer for writing such lousy code that it can only accept %i arguments.", maxArgs);
// 			localError = 1;
// 			break;
// 		}
		
// 		string_substring(&command, &line, 0, string_index_of(&line, 0, ' '));
		
// 		for (int i = 0; i < argc; i++) {
// 			tempIndex = string_index_of(&line, i, ' ');
// 			if (tempIndex == -1) {
// 				log_critical_error(__func__, "Can't happen");
// 				localError = 1;
// 				break;
// 			}
// 			string_substring(&argv[i], &line, tempIndex + 1, string_index_of(&line, i + 1, ' ') - tempIndex);
// 		}
		
// 		if (!string_compare(&command, string_const("newmtl"))) {
// 			/* We have finished loading the material. */
// 			if (acceptCommands) {
// 				break;
// 			}
			
// 			if (argc != 1) {
// 				log_error(__func__, "\"newmtl\" has wrong number of arguments. Should have 1. Line %i", lineNumber);
// 				localError = 1;
// 				goto cleanup_l;
// 			}

// 			if (!string_compare(&argv[0], &obj->material_name)) {
// 				acceptCommands = true;
				
// 			}
// 		}
		
// 		puts(line.value);
// 	}
	
// 	cleanup_l:
	
// 	for (int i = 0; i < maxArgs; i++) {
// 		string_free(&argv[i]);
// 	}
// 	string_free(&command);
// 	string_free(&line);
// 	string_free(&fileText);
// 	string_free(&filePath);
	
// 	return localError;
// }


/* OBJ */
/* === */

// int obj_parsestring(obj_t *obj, const char *string) {
	
// 	unsigned int error = 0;
// 	string_t linecopy;
// 	const char *line = string;
// 	char *newline = NULL;
// 	string_t command;
// 	int tempindex = 0;
// 	string_t argv;
// 	int argc;
// 	string_t argvv;
// 	int argcc;
// 	bool success = true;
	
// 	string_init(&linecopy);
// 	string_init(&command);
// 	string_init(&argv);
// 	string_init(&argvv);
	
// 	for (int linenumber = 1;; linenumber++) {
	
// 		/* Create line. */

// 		/* Copy a line of text into a new string. */
// 		newline = strchr(line, '\n');
// 		if (newline == NULL) {
// 			/* Errors 1 & 2 are taken. */
// 			error = string_copy_c(&linecopy, line);
// 			/* If an error occured, we'll deal with it outside the loop. */
// 			break;
// 		}
// 		string_copy_length_c(&linecopy, line, newline - line);
// 		line = newline+1;

// 		/* Clean line. */
		
// 		/* Remove comments and unnecessary whitespace. */
// 		string_removeLineComments(&linecopy, "#");
// 		/* You see it too, don't you. */
// 		string_removeWhitespace(&linecopy, "melt");
// 		/* Convert remaining whitespace to spaces */
// 		for (int i = 0; i < linecopy.length; i++) {
// 			if (isspace(linecopy.value[i])) {
// 				linecopy.value[i] = ' ';
// 			}
// 		}

// 		/* Ignore empty lines */
// 		if (linecopy.length == 0) {
// 			continue;
// 		}

// 		/* Parse line. */
		
// 		/* Get command */
// 		tempindex = string_index_of(&linecopy, 0, ' ');
// 		if (tempindex < 0) {
// 			log_warning(__func__, "Syntax error | Line %i", linenumber);
// 			success = false;
// 			continue;
// 		}
// 		error = string_substring(&command, &linecopy, 0, tempindex);
// 		if (error) {
// 			break;
// 		}

// 		/* Execute command */
// 		if (!strcmp(command.value, "mtllib")) {
// 			tempindex = string_index_of(&linecopy, 1, ' ');
// 			if (tempindex >= 0) {
// 				log_warning(__func__, "Too many arguments for command %s | Line %i", command.value, linenumber);
// 				success = false;
// 				continue;
// 			}
			
// 			tempindex = string_index_of(&linecopy, 0, ' ');
			
// 			string_substring(&argv, &linecopy, tempindex+1, -1);

// 			string_copy(&obj->material_library, &argv);
// 		}
// 		else if (!strcmp(command.value, "o")) {
// 			tempindex = string_index_of(&linecopy, 1, ' ');
// 			if (tempindex >= 0) {
// 				log_warning(__func__, "Too many arguments for command %s | Line %i", command.value, linenumber);
// 				success = false;
// 				continue;
// 			}
			
// 			tempindex = string_index_of(&linecopy, 0, ' ');
			
// 			string_substring(&argv, &linecopy, tempindex+1, -1);

// 			string_copy(&obj->object_name, &argv);
// 		}
// 		else if (!strcmp(command.value, "v")) {
// 			argc = string_count_char(&linecopy, ' ');
			
// 			if (argc < 3) {
// 				log_warning(__func__, "Too few arguments for command %s | Line %i", command.value, linenumber);
// 				success = false;
// 				continue;
// 			}
			
// 			if (argc > 4) {
// 				log_warning(__func__, "Too many arguments for command %s. Attempting to execute anyway. | Line %i", command.value, linenumber);
// 				success = false;
// 				argc = 4;
// 			}
			
// 			if (obj->geometric_vertices == NULL) {
// 				obj->geometric_vertices = malloc((obj->geometric_vertices_length + 1) * sizeof(vec4_t));
// 			}
// 			else {
// 				/* @TODO: Count needed vertices beforehand to save realloc calls. */
// 				obj->geometric_vertices = realloc(obj->geometric_vertices, (obj->geometric_vertices_length + 1) * sizeof(vec4_t));
// 			}
			
// 			for (int i = 0; i < argc; i++) {
// 				tempindex = string_index_of(&linecopy, i, ' ')+1;
// 				string_substring(&argv, &linecopy, tempindex, string_index_of(&linecopy, i+1, ' ') - tempindex);
				
// 				obj->geometric_vertices[obj->geometric_vertices_length][i] = strtof(argv.value, NULL);
// 			}
			
// 			if (argc == 3) {
// 				obj->geometric_vertices[obj->geometric_vertices_length][3] = 1.0f;
// 			}
			
// 			obj->geometric_vertices_length++;
// 		}
// 		else if (!strcmp(command.value, "vt")) {
// 			argc = string_count_char(&linecopy, ' ');
			
// 			if (argc < 1) {
// 				log_warning(__func__, "Too few arguments for command %s | Line %i", command.value, linenumber);
// 				success = false;
// 				continue;
// 			}
			
// 			if (argc > 3) {
// 				log_warning(__func__, "Too many arguments for command %s. Attempting to execute anyway. | Line %i", command.value, linenumber);
// 				success = false;
// 				argc = 4;
// 			}
			
// 			if (obj->texture_vertices == NULL) {
// 				obj->texture_vertices = malloc((obj->texture_vertices_length + 1) * sizeof(vec4_t));
// 			}
// 			else {
// 				/* @TODO: Count needed vertices beforehand to save realloc calls. */
// 				obj->texture_vertices = realloc(obj->texture_vertices, (obj->texture_vertices_length + 1) * sizeof(vec4_t));
// 			}
			
// 			for (int i = 0; i < argc; i++) {
// 				tempindex = string_index_of(&linecopy, i, ' ')+1;
// 				string_substring(&argv, &linecopy, tempindex, string_index_of(&linecopy, i+1, ' ') - tempindex);
				
// 				obj->texture_vertices[obj->texture_vertices_length][i] = strtof(argv.value, NULL);
// 			}
			
// 			if (argc < 2) {
// 				obj->texture_vertices[obj->texture_vertices_length][1] = 0.0f;
// 			}
			
// 			if (argc < 3) {
// 				obj->texture_vertices[obj->texture_vertices_length][2] = 0.0f;
// 			}
			
// 			obj->texture_vertices_length++;
// 		}
// 		else if (!strcmp(command.value, "vn")) {
// 			argc = string_count_char(&linecopy, ' ');
			
// 			if (argc < 1) {
// 				log_warning(__func__, "Too few arguments for command %s | Line %i", command.value, linenumber);
// 				success = false;
// 				continue;
// 			}
			
// 			if (argc > 3) {
// 				log_warning(__func__, "Too many arguments for command %s. Attempting to execute anyway. | Line %i", command.value, linenumber);
// 				success = false;
// 				argc = 4;
// 			}
			
// 			if (obj->vertex_normals == NULL) {
// 				obj->vertex_normals = malloc((obj->vertex_normals_length + 1) * sizeof(vec4_t));
// 			}
// 			else {
// 				/* @TODO: Count needed vertices beforehand to save realloc calls. */
// 				obj->vertex_normals = realloc(obj->vertex_normals, (obj->vertex_normals_length + 1) * sizeof(vec4_t));
// 			}
			
// 			for (int i = 0; i < argc; i++) {
// 				tempindex = string_index_of(&linecopy, i, ' ')+1;
// 				string_substring(&argv, &linecopy, tempindex, string_index_of(&linecopy, i+1, ' ') - tempindex);
				
// 				obj->vertex_normals[obj->vertex_normals_length][i] = strtof(argv.value, NULL);
// 			}
			
// 			if (argc < 2) {
// 				obj->vertex_normals[obj->vertex_normals_length][1] = 0.0f;
// 			}
			
// 			if (argc < 3) {
// 				obj->vertex_normals[obj->vertex_normals_length][2] = 0.0f;
// 			}
			
// 			obj->vertex_normals_length++;
// 		}
// 		else if (!strcmp(command.value, "usemtl")) {
// 			tempindex = string_index_of(&linecopy, 1, ' ');
// 			if (tempindex >= 0) {
// 				log_warning(__func__, "Too many arguments for command %s | Line %i", command.value, linenumber);
// 				success = false;
// 				continue;
// 			}
			
// 			tempindex = string_index_of(&linecopy, 0, ' ');
			
// 			string_substring(&argv, &linecopy, tempindex+1, -1);

// 			string_copy(&obj->material_name, &argv);
// 		}
// 		else if (!strcmp(command.value, "s")) {
// 			tempindex = string_index_of(&linecopy, 1, ' ');
// 			if (tempindex >= 0) {
// 				log_warning(__func__, "Too many arguments for command %s | Line %i", command.value, linenumber);
// 				success = false;
// 				continue;
// 			}
			
// 			tempindex = string_index_of(&linecopy, 0, ' ');
			
// 			string_substring(&argv, &linecopy, tempindex+1, -1);
			
// 			if (!strcmp(argv.value, "off")) {
// 				obj->smoothing_group = 0;
// 			}
// 			else {
// 				obj->smoothing_group = strtol(argv.value, NULL, 10);
// 				if (obj->smoothing_group < 0) {
// 					log_warning(__func__, "Not sure if negative smoothing groups are permitted. You know what? I'm not doing this. | Line %i", linenumber);
// 					success = false;
// 				}
// 			}
// 		}
// 		else if (!strcmp(command.value, "f")) {
// 			argc = string_count_char(&linecopy, ' ');
			
// 			if (argc < 1) {
// 				log_warning(__func__, "Too few arguments for command %s | Line %i", command.value, linenumber);
// 				success = false;
// 				continue;
// 			}
			
// 			if (obj->facesets == NULL) {
// 				obj->facesets = malloc((obj->facesets_length + 1) * sizeof(faceset_t));
// 			}
// 			else {
// 				/* @TODO: Count needed vertices beforehand to save realloc calls. */
// 				obj->facesets = realloc(obj->facesets, (obj->facesets_length + 1) * sizeof(faceset_t));
// 			}
			
// 			obj->facesets[obj->facesets_length].faces = malloc(argc * sizeof(face_t));
			
// 			for (int i = 0; i < argc; i++) {
// 				tempindex = string_index_of(&linecopy, i, ' ')+1;
// 				string_substring(&argv, &linecopy, tempindex, string_index_of(&linecopy, i+1, ' ') - tempindex);
				
// 				argcc = string_count_char(&argv, '/') + 1;
// 				for (int j = 0; j < argcc; j++) {
// 					tempindex = string_index_of(&argv, j, '/')+1;
// 					string_substring(&argvv, &argv, tempindex, string_index_of(&argv, j+1, '/') - tempindex);

// 					if (j == 0) {
// 						obj->facesets[obj->facesets_length].faces[i].geometric_vertex = strtof(argvv.value, NULL);
// 					}
// 					else if (j == 1) {
// 						obj->facesets[obj->facesets_length].faces[i].texture_vertex = strtof(argvv.value, NULL);
// 					}
// 					else if (j == 2) {
// 						obj->facesets[obj->facesets_length].faces[i].vertex_normal = strtof(argvv.value, NULL);
// 					}
// 				}
				
// 				if (argcc < 2) {
// 					obj->facesets[obj->facesets_length].faces[i].texture_vertex = 0;
// 				}
				
// 				if (argcc < 3) {
// 					obj->facesets[obj->facesets_length].faces[i].vertex_normal = 0;
// 				}
// 			}
			
// 			obj->facesets[obj->facesets_length].faces_length = argc;
// 			obj->facesets_length++;
// 		}
// 		else {
// 			log_warning(__func__, "Unrecognized command \"%s\" | Line %i", command.value, linenumber);
// 			success = false;
// 		}
// 	}
	
// 	/* @TODO: Do we need to check if all faces have the same length? Do we need to do the same with other elements? */
	
// 	string_free(&linecopy);
// 	string_free(&command);
// 	string_free(&argv);
// 	string_free(&argvv);
	
// 	if (error) {
// 		return error;
// 	}
	
// 	if (success) {
		
// 		/* All well and good. Now let's load the material. */
// 		// if ((obj->material_name.length != 0) && (obj->material_library.length != 0)) {
// 		// 	obj_loadMTL(obj);
// 		// }
// 		obj_loadMTL(obj);
	
// 		return 0;
// 	}
// 	else {
// 		log_error(__func__, "Parsing failed");
// 		return 1;
// 	}
// }

// void obj_print(obj_t *obj) {

// 	log_info(__func__, "Dumping object \"%s\"", (obj->object_name.value == NULL) ? "(null)" : obj->object_name.value);

// 	if (obj->material_library.value != NULL) {
// 		printf("Material library "COLOR_BLUE"[mtllib] "COLOR_CYAN"%s"COLOR_NORMAL"\n", obj->material_library.value);
// 	}
	
// 	if (obj->object_name.value != NULL) {
// 		printf("Object name "COLOR_BLUE"[o] "COLOR_CYAN"%s"COLOR_NORMAL"\n", obj->object_name.value);
// 	}
	
// 	for (int i = 0; i < obj->geometric_vertices_length; i++) {
// 		printf("Geometric vertex "COLOR_BLUE"[v]"COLOR_CYAN);
// 		for (int j = 0; j < 4; j++) {
// 			printf(" %+f", obj->geometric_vertices[i][j]);
// 		}
// 		printf(COLOR_NORMAL"\n");
// 	}
	
// 	for (int i = 0; i < obj->texture_vertices_length; i++) {
// 		printf("Texture vertex "COLOR_BLUE"[vt]"COLOR_CYAN);
// 		for (int j = 0; j < 3; j++) {
// 			printf(" %+f", obj->texture_vertices[i][j]);
// 		}
// 		printf(COLOR_NORMAL"\n");
// 	}
	
// 	for (int i = 0; i < obj->vertex_normals_length; i++) {
// 		printf("Vertex normal "COLOR_BLUE"[vn]"COLOR_CYAN);
// 		for (int j = 0; j < 3; j++) {
// 			printf(" %+f", obj->vertex_normals[i][j]);
// 		}
// 		printf(COLOR_NORMAL"\n");
// 	}
	
// 	if (obj->material_name.value != NULL) {
// 		printf("Material name "COLOR_BLUE"[usemtl] "COLOR_CYAN"%s"COLOR_NORMAL"\n", obj->material_name.value);
// 	}
	
// 	printf("Smoothing group "COLOR_BLUE"[s] "COLOR_CYAN"%i"COLOR_NORMAL"\n", obj->smoothing_group);
	
// 	for (int i = 0; i < obj->facesets_length; i++) {
// 		printf("Face "COLOR_BLUE"[f][v/vt/vn]"COLOR_CYAN);
// 		for (int j = 0; j < obj->facesets[i].faces_length; j++) {
// 			// printf(" %i", obj->facesets[i].faces[j].geometric_vertex);
// 			printf(" %i/%i/%i", obj->facesets[i].faces[j].geometric_vertex, obj->facesets[i].faces[j].texture_vertex, obj->facesets[i].faces[j].vertex_normal);
// 		}
// 		printf(COLOR_NORMAL"\n");
// 	}
// }

// int obj_init(obj_t *obj) {
// 	int error = 0;
	
// 	error = string_init(&obj->material_library);
// 	if (error) {
// 		goto cleanup_l;
// 	}
	
// 	string_init(&obj->object_name);
// 	if (error) {
// 		goto cleanup_l;
// 	}
	
// 	string_init(&obj->material_name);
// 	if (error) {
// 		goto cleanup_l;
// 	}
	
// 	obj->geometric_vertices = NULL;
// 	obj->geometric_vertices_length = 0;
// 	obj->texture_vertices = NULL;
// 	obj->texture_vertices_length = 0;
// 	obj->vertex_normals = NULL;
// 	obj->vertex_normals_length = 0;
// 	obj->facesets = NULL;
// 	obj->facesets_length = 0;
// 	obj->smoothing_group = -1;
	
// 	error = ERR_OK;
// 	cleanup_l:
	
// 	return error;
// }

// int obj_free(obj_t *obj) {
// 	string_free(&obj->material_library);
// 	string_free(&obj->object_name);
// 	string_free(&obj->material_name);
// 	free(obj->geometric_vertices);
// 	free(obj->texture_vertices);
// 	free(obj->vertex_normals);
// 	for (int i = 0; i < obj->facesets_length; i++) {
// 		free(obj->facesets[i].faces);
// 	}
// 	free(obj->facesets);
	
// 	return ERR_OK;
// }

// int l_loadObj(lua_State *Lua) {
	
// 	string_t fileName;
// 	string_t filePath;
// 	const char *ext;
// 	string_t fileText;
// 	obj_t obj;
// 	int localError = 0;
	
// 	obj_init(&obj);
	
// 	string_init(&fileText);
// 	string_init(&fileName);
// 	string_init(&filePath);
	
// 	string_copy_c(&fileName, lua_tostring(Lua, 1));
	
// 	/* Don't have to free "ext" because it is part of fileName.value */
// 	ext = file_getExtension(fileName.value);
// 	if (ext == NULL) {
// 		fprintf(stderr, "Error: (l_loadObj) Refusing to open file \"%s\" as Wavefront OBJ due to missing file extension. Should be .obj\n", fileName.value);
// 		localError = 0;
// 		goto cleanup_l;
// 	}
// 	if (strcmp(ext, "obj")) {
// 		fprintf(stderr, "Error: (l_loadObj) Refusing to open file \"%s\" as Wavefront OBJ due to incorrect file extension. Should be .obj\n", fileName.value);
// 		localError = 0;
// 		goto cleanup_l;
// 	}
	
// 	string_copy(&filePath, &fileName);
// 	localError = vfs_getFileText(&g_vfs, &fileText, &filePath);
// 	if (localError) {
// 		goto cleanup_l;
// 	}

// 	log_info(__func__, "Parsing file \"%s\"", filePath.value);
// 	localError = obj_parsestring(&obj, fileText.value);
// 	if (localError) {
// 		log_warning(__func__, "Could not load file \"%s\" as OBJ. (string_to_obj) returned %i", filePath.value, localError);
// 		localError = 0;
// 		goto cleanup_l;
// 	}
// 	// obj_print(&obj);
	
// 	cleanup_l:
	
// 	if (!localError) {
// 		lua_pushstring(Lua, fileText.value);
// 	}
	
// 	string_free(&fileText);
// 	string_free(&fileName);
// 	string_free(&filePath);
	
// 	obj_free(&obj);
	
// 	if (localError) {
// 		return 0;
// 	}
// 	return 1;
// }
