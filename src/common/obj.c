
#include "obj.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "log.h"

static const char *getExt(const char *filename) {
	
	const char *dot = &filename[strlen(filename)];
	
	do {
		--dot;
	} while ((*dot != '.') && (dot > filename));
	
	if (dot == filename)
		return NULL;
	
	return dot+1;
}

static char *getFileText(const char *filename) {
	
	FILE *file = fopen(filename, "r");
	char c;
	string_t str;
	string_init(&str);
	
	while (1) {
		c = fgetc(file);
		if (c == EOF)
			break;
		string_append_char(&str, c);
	}
	
	fclose(file);
	
	return str.value;
}

/* Remove comments and unnecessary whitespace */
int string_beautify(string_t *line, const char linecomment) {

	int error = 0;
	bool sawspace;
	int gap;

	/* Remove comments. */
	error = string_substring(line, line, 0, string_index_of(line, 0, linecomment));
	if (error > 2)
		return error;
	
	/* Remove leading space. */
	sawspace = true;
	/* Remove unnecessary middle whitespace. */
	gap = 0;
        for (int i = 0; i < line->length; i++) {
            if (isspace(line->value[i])) {
                if (sawspace) {
                    gap++;
                }
                else {
                    line->value[i-gap] = ' ';
                    sawspace = true;
                }
            }
            else {
                line->value[i-gap] = line->value[i];
                sawspace = false;
            }
        }
	/* Remove trailing space. */
        if ((line->length+gap > 0) && isspace(line->value[line->length-gap-1]))
            gap++;
        /* Normalize the resulting string since we did a major surgery on it. */
        line->value[line->length-gap] = '\0';
	error = string_normalize(line);
	if (error)
		return error;
		
}

int obj_parsestring(obj_t *obj, const char *string) {
	
	unsigned int error = 0;
	string_t linecopy;
	const char *line = string;
	char *newline = NULL;
	string_t command;
	int tempindex = 0;
	string_t argv;
	int argc;
	/* You know it's bad when: */
	string_t argvv;
	int argcc;
	bool success = true;
	
	string_init(&linecopy);
	string_init(&command);
	string_init(&argv);
	string_init(&argvv);
	
	/* Initialize obj */
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
	
	for (int linenumber = 0;; linenumber++) {
	
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
		error = string_beautify(&linecopy, '#');
		if (error) {
			break;
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
			argc = string_count(&linecopy, ' ');
			
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
			argc = string_count(&linecopy, ' ');
			
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
			argc = string_count(&linecopy, ' ');
			
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
			argc = string_count(&linecopy, ' ');
			
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
				
				argcc = string_count(&argv, '/') + 1;
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
	
	if (success)
		return 0;
	else
		return 1;
}

int print_obj(obj_t *obj) {

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
	
	return 0;
}

int l_loadObj(lua_State *Lua) {
	
	const char *filename = lua_tostring(Lua, 1);
	const char *ext;
	char *filetext;
	obj_t obj;
	int error = 0;
	
	ext = getExt(filename);
	
	if (ext == NULL) {
		fprintf(stderr, "Error: (l_loadObj) Refusing to open file \"%s\" as Wavefront OBJ due to missing file extension. Should be .obj\n", filename);
		return 0;
	}
	
	if (strcmp(ext, "obj")) {
		fprintf(stderr, "Error: (l_loadObj) Refusing to open file \"%s\" as Wavefront OBJ due to incorrect file extension. Should be .obj\n", filename);
		return 0;
	}
	
	filetext = getFileText(filename);
	
	log_info(__func__, "Parsing file \"%s\"", filename);
	error = obj_parsestring(&obj, filetext);
	if (error) {
		log_error(__func__, "Could not load file \"%s\" as OBJ. (string_to_obj) returned %i\n", filename, error);
		free(filetext);
		obj_free(&obj);
		return 0;
	}
	print_obj(&obj);
	
	lua_pushstring(Lua, filetext);
	free(filetext);
	
	obj_free(&obj);
	
	return 1;
}
