
#include "file.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"
#include "insane.h"

char *file_getText(const char *filename) {
	
	FILE *file = fopen(filename, "r");
	char c;
	string_t str;
	
	if (file == NULL) {
		return NULL;
	}
	
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

int file_getLine(string_t *line, const char delimiter, FILE *file) {

	char c;
	int error = 0;
	
	line->length = 0;
	line->value[0] = '\0';
	string_realloc(line);
	
	while (1) {
		c = fgetc(file);
		
		if (c == EOF) {
			return EOF;
		}
		if (c == delimiter) {
			return 0;
		}
		error = string_append_char(line, c);
		if (error)
			return error;
	}
}

int file_exists(const char *filename) {
	
	FILE *file = fopen(filename, "r");
	
	if (file == NULL) {
		return 0;
	}
	
	fclose(file);
	return 1;
}

const char *file_getExtension(const char *filename) {
	
	const char *dot = &filename[strlen(filename)];
	
	do {
		--dot;
	} while ((*dot != '.') && (dot > filename));
	
	if (dot == filename)
		return NULL;
	
	return dot+1;
}

int file_isRegularFile(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode);
}

int file_isDirectory(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

void file_concatenatePath(string_t *destination, const string_t * const source) {

	error = ERR_OK;

	destination->value[destination->length] = '/';
	destination->length++;

	destination->memsize = MSB(destination->length + source->length) + 1;
	destination->value = (char *) realloc(destination->value, (1<<destination->memsize) * sizeof(char));
	if (destination->value == NULL) {
		error = ERR_OUTOFMEMORY;
	}
	
	for (int i = 0; i < source->length; i++) {
		destination->value[i + destination->length] = source->value[i];
	}
	destination->length += source->length;
	destination->value[destination->length] = '\0';

}

void file_getWorkingDirectory(string_t *workingDirectory) {
	free(workingDirectory->value);
	workingDirectory->value = getcwd(NULL, 0);
	string_normalize(workingDirectory);
}

int file_pathStandardize(string_t *filePath) {

	int slashes = 0;
	string_t *parts;
	int *partIndices;
	int partIndicesIndex = 0;
	int localError = 0;
	int tempIndex;
	int dotDots = 0;
	string_t filePathCopy;
	
	string_init(&filePathCopy);

	if (filePath->value[0] != '/') {
		file_getWorkingDirectory(&filePathCopy);
	}
	file_concatenatePath(&filePathCopy, filePath);
	
	slashes = string_count_char(&filePathCopy, '/');
	
	parts = malloc((slashes + 1) * sizeof(string_t));
	partIndices = malloc((slashes + 1) * sizeof(int));

	/*
	Break the path into parts, and then stick them into an array. As that is
	happening, take note of the index and put that into partIndices. At the end
	of the loop, partIndices will have the filePath as an array of ints. "."
	will be ignored when storing indices. ".." will go back to the last index.
	*/
	
	/*
	Break the string into tokens and convert the path into an array of indices.
	At the same time, remove all "." and "..".
	*/
	for (int i = 0; i < slashes + 1; i++) {
	
		string_init(&parts[i]);

		if (i == 0) {
			tempIndex = 0;
		}
		else {
			tempIndex = string_index_of(&filePathCopy, i - 1, '/') + 1;
		}
		string_substring(&parts[i], &filePathCopy, tempIndex, string_index_of(&filePathCopy, i, '/') - tempIndex);

		/* Ignore "". */
		if (!string_compare(&parts[i], string_const(""))) {
			continue;
		}
		/* Ignore ".". */
		if (!string_compare(&parts[i], string_const("."))) {
			continue;
		}
		/* Go back one index if "..". */
		if (!string_compare(&parts[i], string_const(".."))) {
			dotDots++;
			--partIndicesIndex;
			if (partIndicesIndex < 0) {
				localError = 1;
				break;
			}
			continue;
		}
		if (dotDots > 0) {
			--dotDots;
		}
		partIndices[partIndicesIndex] = i;
		partIndicesIndex++;
	}
	
	/* Reassemble the path. */
	for (int i = 0; i < partIndicesIndex; i++) {
		if (i == 0) {
			string_copy(filePath, &parts[partIndices[i]]);
		}
		else {
			file_concatenatePath(filePath, &parts[partIndices[i]]);
		}
	}
	
	for (int i = 0; i < dotDots; i++) {
		file_concatenatePath(filePath, string_const(".."));
	}

	/* Free all. */
	for (int i = 0; i < slashes + 1; i++) {
		string_free(&parts[i]);
	}
	insane_free(parts);
	insane_free(partIndices);
	string_free(&filePathCopy);
	
	return localError;
}

int file_pathIsInDirectory(const string_t * const filePath, const string_t * const workspace) {
	
	string_t filePathCopy;
	string_t workspaceCopy;
	int result;
	/* Sounds so mathematical, but it's not. */
	int divergenceIndex;
	
	string_init(&filePathCopy);
	string_init(&workspaceCopy);
	
	string_copy(&filePathCopy, filePath);
	string_copy(&workspaceCopy, workspace);
	
	file_pathStandardize(&filePathCopy);
	file_pathStandardize(&workspaceCopy);

	if (filePathCopy.length < workspaceCopy.length) {
		result = 0;
		goto cleanup_l;
	}
	
	for (divergenceIndex = 0; divergenceIndex < workspaceCopy.length; divergenceIndex++) {
		if (filePathCopy.value[divergenceIndex] != workspaceCopy.value[divergenceIndex]) {
			result = 0;
			goto cleanup_l;
		}
	}
	
	result = 1;
	
	cleanup_l:
	
	string_free(&filePathCopy);
	string_free(&workspaceCopy);
	
	return result;
}