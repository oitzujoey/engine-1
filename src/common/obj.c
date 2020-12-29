
#include "obj.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "log.h"
#include "vfs.h"
#include "cfg.h"
#include "common.h"
#include "file.h"
#include "insane.h"


/* Model list */
/* ========== */

modelList_t modelList_g;

/* modelList_createModel
model:o         The address of the model.
index:o         The index of the model.
Returns:        error
Globals:        modelList_g
Description:    Creates a model and returns the address and index in the list.
*/
int modelList_createModel(model_t **model, int *index) {
	int error = ERR_OK;

	if (modelList_g.models_length < modelList_g.models_length_actual) {
		modelList_g.models_length++;
	}
	else {
		modelList_g.models_length++;
		modelList_g.models_length_actual++;
		modelList_g.models = realloc(modelList_g.models, modelList_g.models_length_actual * sizeof(model_t));
		if (modelList_g.models == NULL) {
			critical_error("Out of memory", "");
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
	}
	*model = &modelList_g.models[modelList_g.models_length - 1];
	*index = modelList_g.models_length - 1;
	
	model_init(*model);
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

/* modelList_removeLastModel
Globals:        modelList_g
Description:    Ignores the last model that was created.
*/
void modelList_removeLastModel(void) {
	--modelList_g.models_length;
}

/* modelList_free
Globals:        modelList_g
Description:    Frees all models in the list and then frees the list iself.
*/
void modelList_free(void) {
	for (int i = 0; i < modelList_g.models_length_actual; i++) {
		model_free(&modelList_g.models[i]);
	}
	insane_free(modelList_g.models);
	modelList_g.models_length = 0;
	modelList_g.models_length_actual = 0;
}

/* modelList_init
Globals:        modelList_g
Description:    Initialize the model list.
*/
void modelList_init(void) {
	modelList_g.models = NULL;
	modelList_g.models_length = 0;
	modelList_g.models_length_actual = 0;
}

/* Model */
/* ===== */

void model_init(model_t *model) {
	model->faces = NULL;
	model->faces_length = 0;
	model->surface_normals = NULL;
	model->vertices = NULL;
	model->vertices_length = 0;
}

void model_free(model_t *model) {
	insane_free(model->vertices);
	model->vertices_length = 0;
	insane_free(model->surface_normals);
	for (int i = 0; i < model->faces_length; i++) {
		insane_free(model->faces[i]);
	}
	insane_free(model->faces);
	model->faces_length = 0;
}

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
Globals:        modelList_g
Description:    Load an Oolite model from a .dat file and add it into the model
	list. "index" is set to the index of the model in the model list.
*/
int obj_loadOoliteDAT(const string_t *filePath, int *index) {
	int error = ERR_OK;

	string_t fileText;
	string_t line;
	int lineStartIndex;
	int tempIndex0;
	int tempIndex1;
	int datProgress = 0;
	
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
	string_t argv[maxArgs];
	model_t *model;
	int tempModelElementIndex;
	vec3_t tempVec3[2];
	const int progressMask
	    = (1<<mode_start)   | (1<<mode_nverts)  | (1<<mode_nfaces)
	    | (1<<mode_vertex)  | (1<<mode_faces)   | (1<<mode_end);
	
	// model_init(model);
	// Create a model.
	modelList_createModel(&model, index);
	
	datProgress |= 1<<mode_start;
	
	string_init(&fileText);
	string_init(&line);
	for (int i = 0; i < maxArgs; i++) {
		string_init(&argv[i]);
	}

	/* Get text from file. */

	error = vfs_getFileText(&vfs_g, &fileText, filePath);
	if (error) {
		error("vfs_getFileText returned ", ERR[error]);
		error = error;
		goto cleanup_l;
	}
	
	/* Go through each line in file. */
	
	lineStartIndex = 0;

	for (int lineNumber = 1;; lineNumber++) {
		
		/* Get the line. */
		
		// I'm pretty sure this is way more complicated than it need to be.`
		if (lineNumber == 1) {
			tempIndex1 = string_index_of(&fileText, 0, '\n');
			if (tempIndex1 < 0) {
				tempIndex1 = -1;
			}
			else {
				--tempIndex1;
			}
			string_substring(&line, &fileText, 0, tempIndex1);
		}
		else {
			tempIndex0 = string_index_of(&fileText, lineNumber - 2, '\n');
			if (tempIndex0 < 0) {
				// End of file.
				break;
			}
			// Get past the newline.
			tempIndex0++;
			
			tempIndex1 = string_index_of(&fileText, lineNumber - 1, '\n');
			if (tempIndex1 < 0) {
				tempIndex1 = -1;
			}
			else {
				--tempIndex1;
			}
			string_substring(&line, &fileText, tempIndex0, tempIndex1 - tempIndex0);
		}
		
		/* Remove comments and unnecessary whitespace. */
		
		string_removeLineComments(&line, "//");
		string_removeWhitespace(&line, "melt");
		// Convert remaining whitespace to spaces.
		for (int i = 0; i < line.length; i++) {
			if (isspace(line.value[i])) {
				line.value[i] = ' ';
			}
		}

		// Remove empty lines.
		if (!strcmp(line.value, "")) {
			continue;
		}
		
		/* Split line into arguments. */
		
		argc = string_count_char(&line, ' ') + 1;
		if (argc > maxArgs) {
			error("Line %i of file \"%s\" has too many arguments. %i > maxArgs(%i)", lineNumber, filePath->value, argc, maxArgs);
			error = ERR_GENERIC;
			goto cleanup_l;
		}
		
		for (int i = 0; i < argc; i++) {
			
			tempIndex0 = string_index_of(&line, i - 1, ' ');
			tempIndex1 = string_index_of(&line, i, ' ');
			if (i != 0) {
				tempIndex0++;
			}
			error = string_substring(&argv[i], &line, tempIndex0, tempIndex1 - tempIndex0);
			
			// Substring errors are ridiculous. I really need to redo them.
			if (error >= 4) {
				error = ERR_OUTOFMEMORY;
				goto cleanup_l;
			}
			if (error) {
				error = ERR_GENERIC;
				goto cleanup_l;
			}
		}
		
		/* Execute command if one exists. */
		
		if (!strcmp(argv[0].value, "NVERTS")) {
			
			if (argc != 2) {
				error("\"%s\":%i \"%s\" requires 2 arguments", filePath->value, lineNumber, argv[0].value);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			model->vertices_length = strtol(argv[1].value, NULL, 10);
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
		if (!strcmp(argv[0].value, "NFACES")) {
			
			if (argc != 2) {
				error("\"%s\":%i \"%s\" requires 2 arguments", filePath->value, lineNumber, argv[0].value);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			model->faces_length = strtol(argv[1].value, NULL, 10);
			// I should really be checking for INT_MAX and INT_MIN.
			
			if (model->faces_length != 0) {
				model->faces = malloc(model->faces_length * sizeof(int *));
				for (int i = 0; i < model->faces_length; i++) {
					model->faces[i] = malloc(3 * sizeof(int));
				}
			}
			else {
				model->faces = NULL;
			}
			
			// Does nothing special.
			mode = mode_nfaces;
			datProgress |= 1<<mode_nfaces;
			continue;
		}
		if (!strcmp(argv[0].value, "VERTEX")) {
			
			if (argc != 1) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath->value, lineNumber, argv[0].value);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			if (((1<<mode_nverts) & datProgress) == 0) {
				error("\"%s\":%i NVERTS has not been declared", filePath->value, lineNumber);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			tempModelElementIndex = 0;
			
			mode = mode_vertex;
			// Don't set progress until the last vertex has been set.
			// datProgress |= 1<<mode_vertex;
			continue;
		}
		if (!strcmp(argv[0].value, "FACES")) {
			
			if (argc != 1) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath->value, lineNumber, argv[0].value);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			if (((1<<mode_nfaces) & datProgress) == 0) {
				error("\"%s\":%i NFACES has not been declared", filePath->value, lineNumber);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			tempModelElementIndex = 0;
			
			mode = mode_faces;
			// Don't set progress until the last face has been set.
			// datProgress |= 1<<mode_faces;
			continue;
		}
		if (!strcmp(argv[0].value, "TEXTURES")) {
			
			if (argc != 1) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath->value, lineNumber, argv[0].value);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			// @TODO: Do something about the textures.
			
			tempModelElementIndex = 0;
			
			mode = mode_textures;
			continue;
		}
		if (!strcmp(argv[0].value, "NAMES")) {
			
			if (argc != 2) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath->value, lineNumber, argv[0].value);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			// @TODO: Do something about the names.
			
			tempModelElementIndex = 0;
			
			mode = mode_names;
			continue;
		}
		if (!strcmp(argv[0].value, "NORMALS")) {
			
			if (argc != 1) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath->value, lineNumber, argv[0].value);
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			// @TODO: Do something about the normals.
			
			tempModelElementIndex = 0;
			
			mode = mode_normals;
			continue;
		}
		if (!strcmp(argv[0].value, "END")) {
			
			// Picky, picky.
			if (argc != 1) {
				error("\"%s\":%i \"%s\" requires 1 argument", filePath->value, lineNumber, argv[0].value);
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
					error("\"%s\":%i mode \"vertex\" requires 3 arguments", filePath->value, lineNumber);
					error = ERR_GENERIC;
					goto cleanup_l;
				}
				
				if (tempModelElementIndex >= model->vertices_length) {
					error("\"%s\":%i Vertex list is longer than declared by NVERTS(%i)", filePath->value, lineNumber, model->vertices_length);
					error = ERR_GENERIC;
					goto cleanup_l;
				}
				
				for (int i = 0; i < 3; i++) {
					model->vertices[tempModelElementIndex][i] = strtof(argv[i].value, NULL);
				}
				tempModelElementIndex++;
				
				if (tempModelElementIndex == model->vertices_length) {
					// Done.
					datProgress |= 1<<mode_vertex;
				}
				
				continue;
			
			case mode_faces:
				
				if (argc != 10) {
					error("\"%s\":%i mode \"faces\" requires 10 arguments", filePath->value, lineNumber);
					error = ERR_GENERIC;
					goto cleanup_l;
				}
				
				if (tempModelElementIndex >= model->faces_length) {
					error("\"%s\":%i Face list is longer than declared by NFACES(%i)", filePath->value, lineNumber, model->faces_length);
					error = ERR_GENERIC;
					goto cleanup_l;
				}
				
				for (int i = 0; i < 3; i++) {
					model->faces[tempModelElementIndex][i] = strtof(argv[i + 7].value, NULL);
				}
				tempModelElementIndex++;
				
				if (tempModelElementIndex == model->faces_length) {
					// Done.
					datProgress |= 1<<mode_faces;
				}
				
				continue;
			
			case mode_textures:
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
		error("\"%s\" Model not fully defined by file", filePath->value);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Create suface normals.
	model->surface_normals = malloc(model->faces_length * sizeof(vec3_t));
	if (model->surface_normals == NULL) {
		critical_error("Out of memory", "");
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
	
	error = ERR_OK;
	cleanup_l:
	
	if (error) {
		model_free(model);
		modelList_removeLastModel();
	}
	
	for (int i = 0; i < maxArgs; i++) {
		string_free(&argv[i]);
	}
	string_free(&line);
	string_free(&fileText);
	
	return error;
}

int l_obj_loadOoliteDAT(lua_State *luaState) {
	int error = 0;
	
	int index = -1;
	string_t filePath;
	
	string_init(&filePath);
	
	if (!lua_isstring(luaState, 1)) {
		error("Argument 1 must be a string.", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	string_copy_c(&filePath, lua_tostring(luaState, 1));
	
	error = obj_loadOoliteDAT(&filePath, &index);
	if (error) {
		goto cleanup_l;
	}
	
	error = 0;
	cleanup_l:
	
	string_free(&filePath);
	
	lua_pushinteger(luaState, index);
	lua_pushinteger(luaState, error);
	
	return 2;
}

/* MTL */
/* === */

static int mtl_parsestring(mtl_t *mtl, const char *string) {
}

static int mtl_print(mtl_t *mtl) {
}

static int mtl_init(mtl_t *mtl) {
}

static int mtl_free(mtl_t *mtl) {
}

int obj_loadMTL(obj_t *obj) {
	
	string_t fileText;
	string_t filePath;
	int localError = 0;
	int materialsInFile = 0;
	/* This is not really a string. It should never be init or freed. */
	string_t line;
	int newlineIndex;
	int tempIndex;
	bool acceptCommands = false;
	string_t command;
	int argc;
	const int maxArgs = 3;
	string_t argv[maxArgs];
	
	string_init(&fileText);
	string_init(&filePath);
	string_init(&line);
	string_init(&command);
	for (int i = 0; i < maxArgs; i++) {
		string_init(&argv[i]);
	}
	
	if ((obj->material_name.length == 0) || (obj->material_library.length == 0)) {
		return ERR_GENERIC;
	}
	
	string_copy(&filePath, &obj->material_library);
	
	log_info(__func__, "Loading material \"%s\"", filePath.value);
	
	localError = vfs_getFileText(&vfs_g, &fileText, &filePath);
	if (localError) {
		log_error(__func__, "Could not open file \"%s\"", filePath.value);
		goto cleanup_l;
	}
	
	for (int lineNumber = 1;; lineNumber++) {
		
		tempIndex = string_index_of(&fileText, lineNumber - 1, '\n');
		if (tempIndex == -1) {
			break;
		}
		tempIndex++;
			
		string_substring(&line, &fileText, tempIndex, string_index_of(&fileText, lineNumber, '\n') - tempIndex);
		
		/* Remove comments and unnecessary whitespace. */
		string_removeLineComments(&line, "#");
		string_removeWhitespace(&line, "melt");
		/* Convert remaining whitespace to spaces */
		for (int i = 0; i < line.length; i++) {
			if (isspace(line.value[i])) {
				line.value[i] = ' ';
			}
		}

		/* Ignore empty lines */
		if (line.length == 0) {
			continue;
		}

		argc = string_count_char(&line, ' ');
		if (argc > maxArgs) {
			log_critical_error(__func__, "Please reprimand the developer for writing such lousy code that it can only accept %i arguments.", maxArgs);
			localError = 1;
			break;
		}
		
		string_substring(&command, &line, 0, string_index_of(&line, 0, ' '));
		
		for (int i = 0; i < argc; i++) {
			tempIndex = string_index_of(&line, i, ' ');
			if (tempIndex == -1) {
				log_critical_error(__func__, "Can't happen");
				localError = 1;
				break;
			}
			string_substring(&argv[i], &line, tempIndex + 1, string_index_of(&line, i + 1, ' ') - tempIndex);
		}
		
		if (!string_compare(&command, string_const("newmtl"))) {
			/* We have finished loading the material. */
			if (acceptCommands) {
				break;
			}
			
			if (argc != 1) {
				log_error(__func__, "\"newmtl\" has wrong number of arguments. Should have 1. Line %i", lineNumber);
				localError = 1;
				goto cleanup_l;
			}

			if (!string_compare(&argv[0], &obj->material_name)) {
				acceptCommands = true;
				
			}
		}
		
		puts(line.value);
	}
	
	cleanup_l:
	
	for (int i = 0; i < maxArgs; i++) {
		string_free(&argv[i]);
	}
	string_free(&command);
	string_free(&line);
	string_free(&fileText);
	string_free(&filePath);
	
	return localError;
}


/* OBJ */
/* === */

int obj_parsestring(obj_t *obj, const char *string) {
	
	unsigned int error = 0;
	string_t linecopy;
	const char *line = string;
	char *newline = NULL;
	string_t command;
	int tempindex = 0;
	string_t argv;
	int argc;
	string_t argvv;
	int argcc;
	bool success = true;
	
	string_init(&linecopy);
	string_init(&command);
	string_init(&argv);
	string_init(&argvv);
	
	for (int linenumber = 1;; linenumber++) {
	
		/* Create line. */

		/* Copy a line of text into a new string. */
		newline = strchr(line, '\n');
		if (newline == NULL) {
			/* Errors 1 & 2 are taken. */
			error = string_copy_c(&linecopy, line);
			/* If an error occured, we'll deal with it outside the loop. */
			break;
		}
		string_copy_length_c(&linecopy, line, newline - line);
		line = newline+1;

		/* Clean line. */
		
		/* Remove comments and unnecessary whitespace. */
		string_removeLineComments(&linecopy, "#");
		/* You see it too, don't you. */
		string_removeWhitespace(&linecopy, "melt");
		/* Convert remaining whitespace to spaces */
		for (int i = 0; i < linecopy.length; i++) {
			if (isspace(linecopy.value[i])) {
				linecopy.value[i] = ' ';
			}
		}

		/* Ignore empty lines */
		if (linecopy.length == 0) {
			continue;
		}

		/* Parse line. */
		
		/* Get command */
		tempindex = string_index_of(&linecopy, 0, ' ');
		if (tempindex < 0) {
			log_warning(__func__, "Syntax error | Line %i", linenumber);
			success = false;
			continue;
		}
		error = string_substring(&command, &linecopy, 0, tempindex);
		if (error) {
			break;
		}

		/* Execute command */
		if (!strcmp(command.value, "mtllib")) {
			tempindex = string_index_of(&linecopy, 1, ' ');
			if (tempindex >= 0) {
				log_warning(__func__, "Too many arguments for command %s | Line %i", command.value, linenumber);
				success = false;
				continue;
			}
			
			tempindex = string_index_of(&linecopy, 0, ' ');
			
			string_substring(&argv, &linecopy, tempindex+1, -1);

			string_copy(&obj->material_library, &argv);
		}
		else if (!strcmp(command.value, "o")) {
			tempindex = string_index_of(&linecopy, 1, ' ');
			if (tempindex >= 0) {
				log_warning(__func__, "Too many arguments for command %s | Line %i", command.value, linenumber);
				success = false;
				continue;
			}
			
			tempindex = string_index_of(&linecopy, 0, ' ');
			
			string_substring(&argv, &linecopy, tempindex+1, -1);

			string_copy(&obj->object_name, &argv);
		}
		else if (!strcmp(command.value, "v")) {
			argc = string_count_char(&linecopy, ' ');
			
			if (argc < 3) {
				log_warning(__func__, "Too few arguments for command %s | Line %i", command.value, linenumber);
				success = false;
				continue;
			}
			
			if (argc > 4) {
				log_warning(__func__, "Too many arguments for command %s. Attempting to execute anyway. | Line %i", command.value, linenumber);
				success = false;
				argc = 4;
			}
			
			if (obj->geometric_vertices == NULL) {
				obj->geometric_vertices = malloc((obj->geometric_vertices_length + 1) * sizeof(vec4_t));
			}
			else {
				/* @TODO: Count needed vertices beforehand to save realloc calls. */
				obj->geometric_vertices = realloc(obj->geometric_vertices, (obj->geometric_vertices_length + 1) * sizeof(vec4_t));
			}
			
			for (int i = 0; i < argc; i++) {
				tempindex = string_index_of(&linecopy, i, ' ')+1;
				string_substring(&argv, &linecopy, tempindex, string_index_of(&linecopy, i+1, ' ') - tempindex);
				
				obj->geometric_vertices[obj->geometric_vertices_length][i] = strtof(argv.value, NULL);
			}
			
			if (argc == 3) {
				obj->geometric_vertices[obj->geometric_vertices_length][3] = 1.0f;
			}
			
			obj->geometric_vertices_length++;
		}
		else if (!strcmp(command.value, "vt")) {
			argc = string_count_char(&linecopy, ' ');
			
			if (argc < 1) {
				log_warning(__func__, "Too few arguments for command %s | Line %i", command.value, linenumber);
				success = false;
				continue;
			}
			
			if (argc > 3) {
				log_warning(__func__, "Too many arguments for command %s. Attempting to execute anyway. | Line %i", command.value, linenumber);
				success = false;
				argc = 4;
			}
			
			if (obj->texture_vertices == NULL) {
				obj->texture_vertices = malloc((obj->texture_vertices_length + 1) * sizeof(vec4_t));
			}
			else {
				/* @TODO: Count needed vertices beforehand to save realloc calls. */
				obj->texture_vertices = realloc(obj->texture_vertices, (obj->texture_vertices_length + 1) * sizeof(vec4_t));
			}
			
			for (int i = 0; i < argc; i++) {
				tempindex = string_index_of(&linecopy, i, ' ')+1;
				string_substring(&argv, &linecopy, tempindex, string_index_of(&linecopy, i+1, ' ') - tempindex);
				
				obj->texture_vertices[obj->texture_vertices_length][i] = strtof(argv.value, NULL);
			}
			
			if (argc < 2) {
				obj->texture_vertices[obj->texture_vertices_length][1] = 0.0f;
			}
			
			if (argc < 3) {
				obj->texture_vertices[obj->texture_vertices_length][2] = 0.0f;
			}
			
			obj->texture_vertices_length++;
		}
		else if (!strcmp(command.value, "vn")) {
			argc = string_count_char(&linecopy, ' ');
			
			if (argc < 1) {
				log_warning(__func__, "Too few arguments for command %s | Line %i", command.value, linenumber);
				success = false;
				continue;
			}
			
			if (argc > 3) {
				log_warning(__func__, "Too many arguments for command %s. Attempting to execute anyway. | Line %i", command.value, linenumber);
				success = false;
				argc = 4;
			}
			
			if (obj->vertex_normals == NULL) {
				obj->vertex_normals = malloc((obj->vertex_normals_length + 1) * sizeof(vec4_t));
			}
			else {
				/* @TODO: Count needed vertices beforehand to save realloc calls. */
				obj->vertex_normals = realloc(obj->vertex_normals, (obj->vertex_normals_length + 1) * sizeof(vec4_t));
			}
			
			for (int i = 0; i < argc; i++) {
				tempindex = string_index_of(&linecopy, i, ' ')+1;
				string_substring(&argv, &linecopy, tempindex, string_index_of(&linecopy, i+1, ' ') - tempindex);
				
				obj->vertex_normals[obj->vertex_normals_length][i] = strtof(argv.value, NULL);
			}
			
			if (argc < 2) {
				obj->vertex_normals[obj->vertex_normals_length][1] = 0.0f;
			}
			
			if (argc < 3) {
				obj->vertex_normals[obj->vertex_normals_length][2] = 0.0f;
			}
			
			obj->vertex_normals_length++;
		}
		else if (!strcmp(command.value, "usemtl")) {
			tempindex = string_index_of(&linecopy, 1, ' ');
			if (tempindex >= 0) {
				log_warning(__func__, "Too many arguments for command %s | Line %i", command.value, linenumber);
				success = false;
				continue;
			}
			
			tempindex = string_index_of(&linecopy, 0, ' ');
			
			string_substring(&argv, &linecopy, tempindex+1, -1);

			string_copy(&obj->material_name, &argv);
		}
		else if (!strcmp(command.value, "s")) {
			tempindex = string_index_of(&linecopy, 1, ' ');
			if (tempindex >= 0) {
				log_warning(__func__, "Too many arguments for command %s | Line %i", command.value, linenumber);
				success = false;
				continue;
			}
			
			tempindex = string_index_of(&linecopy, 0, ' ');
			
			string_substring(&argv, &linecopy, tempindex+1, -1);
			
			if (!strcmp(argv.value, "off")) {
				obj->smoothing_group = 0;
			}
			else {
				obj->smoothing_group = strtol(argv.value, NULL, 10);
				if (obj->smoothing_group < 0) {
					log_warning(__func__, "Not sure if negative smoothing groups are permitted. You know what? I'm not doing this. | Line %i", linenumber);
					success = false;
				}
			}
		}
		else if (!strcmp(command.value, "f")) {
			argc = string_count_char(&linecopy, ' ');
			
			if (argc < 1) {
				log_warning(__func__, "Too few arguments for command %s | Line %i", command.value, linenumber);
				success = false;
				continue;
			}
			
			if (obj->facesets == NULL) {
				obj->facesets = malloc((obj->facesets_length + 1) * sizeof(faceset_t));
			}
			else {
				/* @TODO: Count needed vertices beforehand to save realloc calls. */
				obj->facesets = realloc(obj->facesets, (obj->facesets_length + 1) * sizeof(faceset_t));
			}
			
			obj->facesets[obj->facesets_length].faces = malloc(argc * sizeof(face_t));
			
			for (int i = 0; i < argc; i++) {
				tempindex = string_index_of(&linecopy, i, ' ')+1;
				string_substring(&argv, &linecopy, tempindex, string_index_of(&linecopy, i+1, ' ') - tempindex);
				
				argcc = string_count_char(&argv, '/') + 1;
				for (int j = 0; j < argcc; j++) {
					tempindex = string_index_of(&argv, j, '/')+1;
					string_substring(&argvv, &argv, tempindex, string_index_of(&argv, j+1, '/') - tempindex);

					if (j == 0) {
						obj->facesets[obj->facesets_length].faces[i].geometric_vertex = strtof(argvv.value, NULL);
					}
					else if (j == 1) {
						obj->facesets[obj->facesets_length].faces[i].texture_vertex = strtof(argvv.value, NULL);
					}
					else if (j == 2) {
						obj->facesets[obj->facesets_length].faces[i].vertex_normal = strtof(argvv.value, NULL);
					}
				}
				
				if (argcc < 2) {
					obj->facesets[obj->facesets_length].faces[i].texture_vertex = 0;
				}
				
				if (argcc < 3) {
					obj->facesets[obj->facesets_length].faces[i].vertex_normal = 0;
				}
			}
			
			obj->facesets[obj->facesets_length].faces_length = argc;
			obj->facesets_length++;
		}
		else {
			log_warning(__func__, "Unrecognized command \"%s\" | Line %i", command.value, linenumber);
			success = false;
		}
	}
	
	/* @TODO: Do we need to check if all faces have the same length? Do we need to do the same with other elements? */
	
	string_free(&linecopy);
	string_free(&command);
	string_free(&argv);
	string_free(&argvv);
	
	if (error) {
		return error;
	}
	
	if (success) {
		
		/* All well and good. Now let's load the material. */
		// if ((obj->material_name.length != 0) && (obj->material_library.length != 0)) {
		// 	obj_loadMTL(obj);
		// }
		obj_loadMTL(obj);
	
		return 0;
	}
	else {
		log_error(__func__, "Parsing failed");
		return 1;
	}
}

int obj_print(obj_t *obj) {

	log_info(__func__, "Dumping object \"%s\"", (obj->object_name.value == NULL) ? "(null)" : obj->object_name.value);

	if (obj->material_library.value != NULL) {
		printf("Material library "COLOR_BLUE"[mtllib] "COLOR_CYAN"%s"COLOR_NORMAL"\n", obj->material_library.value);
	}
	
	if (obj->object_name.value != NULL) {
		printf("Object name "COLOR_BLUE"[o] "COLOR_CYAN"%s"COLOR_NORMAL"\n", obj->object_name.value);
	}
	
	for (int i = 0; i < obj->geometric_vertices_length; i++) {
		printf("Geometric vertex "COLOR_BLUE"[v]"COLOR_CYAN);
		for (int j = 0; j < 4; j++) {
			printf(" %+f", obj->geometric_vertices[i][j]);
		}
		printf(COLOR_NORMAL"\n");
	}
	
	for (int i = 0; i < obj->texture_vertices_length; i++) {
		printf("Texture vertex "COLOR_BLUE"[vt]"COLOR_CYAN);
		for (int j = 0; j < 3; j++) {
			printf(" %+f", obj->texture_vertices[i][j]);
		}
		printf(COLOR_NORMAL"\n");
	}
	
	for (int i = 0; i < obj->vertex_normals_length; i++) {
		printf("Vertex normal "COLOR_BLUE"[vn]"COLOR_CYAN);
		for (int j = 0; j < 3; j++) {
			printf(" %+f", obj->vertex_normals[i][j]);
		}
		printf(COLOR_NORMAL"\n");
	}
	
	if (obj->material_name.value != NULL) {
		printf("Material name "COLOR_BLUE"[usemtl] "COLOR_CYAN"%s"COLOR_NORMAL"\n", obj->material_name.value);
	}
	
	printf("Smoothing group "COLOR_BLUE"[s] "COLOR_CYAN"%i"COLOR_NORMAL"\n", obj->smoothing_group);
	
	for (int i = 0; i < obj->facesets_length; i++) {
		printf("Face "COLOR_BLUE"[f][v/vt/vn]"COLOR_CYAN);
		for (int j = 0; j < obj->facesets[i].faces_length; j++) {
			// printf(" %i", obj->facesets[i].faces[j].geometric_vertex);
			printf(" %i/%i/%i", obj->facesets[i].faces[j].geometric_vertex, obj->facesets[i].faces[j].texture_vertex, obj->facesets[i].faces[j].vertex_normal);
		}
		printf(COLOR_NORMAL"\n");
	}
}

int obj_init(obj_t *obj) {
	string_init(&obj->material_library);
	string_init(&obj->object_name);
	string_init(&obj->material_name);
	obj->geometric_vertices = NULL;
	obj->geometric_vertices_length = 0;
	obj->texture_vertices = NULL;
	obj->texture_vertices_length = 0;
	obj->vertex_normals = NULL;
	obj->vertex_normals_length = 0;
	obj->facesets = NULL;
	obj->facesets_length = 0;
	obj->smoothing_group = -1;
}

int obj_free(obj_t *obj) {
	string_free(&obj->material_library);
	string_free(&obj->object_name);
	string_free(&obj->material_name);
	free(obj->geometric_vertices);
	free(obj->texture_vertices);
	free(obj->vertex_normals);
	for (int i = 0; i < obj->facesets_length; i++) {
		free(obj->facesets[i].faces);
	}
	free(obj->facesets);
	
	return ERR_OK;
}

int l_loadObj(lua_State *Lua) {
	
	string_t fileName;
	string_t filePath;
	const char *ext;
	string_t fileText;
	obj_t obj;
	int localError = 0;
	
	obj_init(&obj);
	
	string_init(&fileText);
	string_init(&fileName);
	string_init(&filePath);
	
	string_copy_c(&fileName, lua_tostring(Lua, 1));
	
	/* Don't have to free "ext" because it is part of fileName.value */
	ext = file_getExtension(fileName.value);
	if (ext == NULL) {
		fprintf(stderr, "Error: (l_loadObj) Refusing to open file \"%s\" as Wavefront OBJ due to missing file extension. Should be .obj\n", fileName.value);
		localError = 0;
		goto cleanup_l;
	}
	if (strcmp(ext, "obj")) {
		fprintf(stderr, "Error: (l_loadObj) Refusing to open file \"%s\" as Wavefront OBJ due to incorrect file extension. Should be .obj\n", fileName.value);
		localError = 0;
		goto cleanup_l;
	}
	
	string_copy(&filePath, &fileName);
	localError = vfs_getFileText(&vfs_g, &fileText, &filePath);
	if (localError) {
		goto cleanup_l;
	}

	log_info(__func__, "Parsing file \"%s\"", filePath.value);
	localError = obj_parsestring(&obj, fileText.value);
	if (localError) {
		log_warning(__func__, "Could not load file \"%s\" as OBJ. (string_to_obj) returned %i", filePath.value, localError);
		localError = 0;
		goto cleanup_l;
	}
	// obj_print(&obj);
	
	cleanup_l:
	
	if (!localError) {
		lua_pushstring(Lua, fileText.value);
	}
	
	string_free(&fileText);
	string_free(&fileName);
	string_free(&filePath);
	
	obj_free(&obj);
	
	if (localError) {
		return 0;
	}
	return 1;
}
