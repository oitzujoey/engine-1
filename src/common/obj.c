
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
