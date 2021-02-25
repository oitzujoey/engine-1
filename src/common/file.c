
#include "file.h"
#include <string.h>
#include <stdlib.h>
#include <physfs.h>
#include "common.h"
#include "insane.h"
#include "str2.h"

// char *file_getText(const char *filename) {
// 	int 

// 	#error FIXME!!!
// 	PHYSFS_File *file = PHYSFS_openRead(filename);
// 	// FILE *file = fopen(filename, "r");
// 	char c;
// 	char *str = NULL;
	
// 	if (file == NULL) {
// 		return NULL;
// 	}
	
// 	PHYSFS_sint64 fileTextLength = PHYSFS_fileLength(file);
// 	error = str2_realloc(str, fileTextLength);
// 	if (str == NULL)
	
// 	PHYSFS_readBytes(file, str, fileTextLength);
	
// 	while (1) {
// 		c = fgetc(file);
// 		if (c == EOF)
// 			break;
// 		#error Fix this function.
// 		// string_append_char(&str, c);
// 	}
	
// 	fclose(file);
	
// 	return str;
// }

// int file_getLine(char *line, const char delimiter, FILE *file) {
// 	int error = ERR_CRITICAL;

// 	char c;
	
// 	line[0] = '\0';
// 	error = str2_realloc(line, 0);
// 	if (error) {
// 		error = ERR_OUTOFMEMORY;
// 		goto cleanup_l;
// 	}
	
// 	while (1) {
// 		c = fgetc(file);
		
// 		if (c == EOF) {
// 			return EOF;
// 		}
// 		if (c == delimiter) {
// 			return 0;
// 		}
// 		// error = string_append_char(line, c);
// 		#error Fix this function.
// 		if (error)
// 			return error;
// 	}
	
// 	error = ERR_OK;
// 	cleanup_l:
	
// 	return error;
// }

// int file_exists(const char *filename) {
// 	#error FIXME!!!
// 	FILE *file = fopen(filename, "r");
	
// 	if (file == NULL) {
// 		return 0;
// 	}
	
// 	fclose(file);
// 	return 1;
// }

const char *file_getExtension(const char *filename) {
	
	const char *dot = &filename[strlen(filename)];
	
	do {
		--dot;
	} while ((*dot != '.') && (dot > filename));
	
	if (dot == filename)
		return NULL;
	
	return dot+1;
}

// void file_isRegularFile(const char *path) {
// 	// struct stat path_stat;
// 	// stat(path, &path_stat);
// 	// return S_ISREG(path_stat.st_mode);
// 	#error FIXME!!!
// }

// void file_isDirectory(const char *path) {
// 	// struct stat path_stat;
// 	// stat(path, &path_stat);
// 	// return S_ISDIR(path_stat.st_mode);
// 	#error FIXME!!!
// }

int file_concatenatePath(char **destination, const char *source) {
	int error = ERR_OK;
	
	size_t destination_length = strlen(*destination);
	size_t source_length = strlen(source);

	(*destination)[destination_length] = '/';
	destination_length++;

	error = str2_realloc(destination, destination_length + source_length);
	if (error) {
		goto cleanup_l;
	}
	
	for (int i = 0; i < source_length; i++) {
		(*destination)[i + destination_length] = source[i];
	}
	destination_length += source_length;
	(*destination)[destination_length] = '\0';
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

void file_getWorkingDirectory(char *workingDirectory) {
	free(workingDirectory);
	workingDirectory = getcwd(NULL, 0);
}

void file_resolveRelativePaths(char *path) {
	// int error = ERR_CRITICAL;
	
	size_t numDeletions = 0;
	size_t path_length = strlen(path);
	size_t tokens_length = 1;
	char *token = path;
	char *copyPointer = path + strlen(path);
	char *copyTop = NULL;
	
	// "Tokenize" string.
	for (int i = 0; i < path_length; i++) {
		if (path[i] == '/') {
			path[i] = '\0';
			tokens_length++;
		}
	}
	
	// Go to last token.
	token = path + path_length - 1;
	
	for (int i = tokens_length - 1; i >= 0; --i) {
		
		// Go to beginning of token.
		while ((*token != '\0') && (token > path)) {
			--token;
		}
		if (*token == '\0') {
			token++;
		}
		
		if (!strcmp(token, "..")) {
			numDeletions++;
		}
		else if (numDeletions > 0) {
			--numDeletions;
		}
		else {
			// Build file path starting from end of string.
			for (int j = strlen(token) - 1; j >= 0; --j) {
				*--copyPointer = token[j];
			}
			
			// Path separator.
			*--copyPointer = '\0';
		}
		
		// Go back a token.
		token -= 2;
	}
	token = NULL;
	
	// Start moving from here.
	// <= so we get the '\0'.
	copyPointer++;
	copyTop = path + path_length;
	path_length = path + path_length - copyPointer;
	for (int i = 0; copyPointer <= copyTop; i++) {
		path[i] = *copyPointer++;
		if (path[i] == '\0') {
			path[i] = '/';
		}
	}
	path[path_length] = '\0';
	
	// cleanup_l:
	
	// return error;
}

// int file_pathStandardize(char *filePath) {

// 	int slashes = 0;
// 	char **parts;
// 	int *partIndices;
// 	int partIndicesIndex = 0;
// 	int localError = 0;
// 	int tempIndex;
// 	int dotDots = 0;
// 	char *filePathCopy = NULL;
	
// 	if (filePath[0] != '/') {
// 		file_getWorkingDirectory(&filePathCopy);
// 	}
// 	file_concatenatePath(&filePathCopy, filePath);
	
// 	slashes = string_count_char(&filePathCopy, '/');
	
// 	parts = malloc((slashes + 1) * sizeof(char *));
// 	partIndices = malloc((slashes + 1) * sizeof(int));

// 	/*
// 	Break the path into parts, and then stick them into an array. As that is
// 	happening, take note of the index and put that into partIndices. At the end
// 	of the loop, partIndices will have the filePath as an array of ints. "."
// 	will be ignored when storing indices. ".." will go back to the last index.
// 	*/
	
// 	/*
// 	Break the string into tokens and convert the path into an array of indices.
// 	At the same time, remove all "." and "..".
// 	*/
// 	for (int i = 0; i < slashes + 1; i++) {
	
// 		parts[i] = NULL

// 		if (i == 0) {
// 			tempIndex = 0;
// 		}
// 		else {
// 			tempIndex = string_index_of(&filePathCopy, i - 1, '/') + 1;
// 		}
// 		string_substring(&parts[i], &filePathCopy, tempIndex, string_index_of(&filePathCopy, i, '/') - tempIndex);

// 		/* Ignore "". */
// 		if (!string_compare(&parts[i], string_const(""))) {
// 			continue;
// 		}
// 		/* Ignore ".". */
// 		if (!string_compare(&parts[i], string_const("."))) {
// 			continue;
// 		}
// 		/* Go back one index if "..". */
// 		if (!string_compare(&parts[i], string_const(".."))) {
// 			dotDots++;
// 			--partIndicesIndex;
// 			if (partIndicesIndex < 0) {
// 				localError = 1;
// 				break;
// 			}
// 			continue;
// 		}
// 		if (dotDots > 0) {
// 			--dotDots;
// 		}
// 		partIndices[partIndicesIndex] = i;
// 		partIndicesIndex++;
// 	}
	
// 	/* Reassemble the path. */
// 	for (int i = 0; i < partIndicesIndex; i++) {
// 		if (i == 0) {
// 			string_copy(filePath, &parts[partIndices[i]]);
// 		}
// 		else {
// 			file_concatenatePath(filePath, &parts[partIndices[i]]);
// 		}
// 	}
	
// 	for (int i = 0; i < dotDots; i++) {
// 		file_concatenatePath(filePath, string_const(".."));
// 	}

// 	/* Free all. */
// 	for (int i = 0; i < slashes + 1; i++) {
// 		string_free(&parts[i]);
// 	}
// 	insane_free(parts);
// 	insane_free(partIndices);
// 	string_free(&filePathCopy);
	
// 	return localError;
// }

// int file_pathIsInDirectory(const string_t * const filePath, const string_t * const workspace) {
	
// 	string_t filePathCopy;
// 	string_t workspaceCopy;
// 	int result;
// 	/* Sounds so mathematical, but it's not. */
// 	int divergenceIndex;
	
// 	string_init(&filePathCopy);
// 	string_init(&workspaceCopy);
	
// 	string_copy(&filePathCopy, filePath);
// 	string_copy(&workspaceCopy, workspace);
	
// 	file_pathStandardize(&filePathCopy);
// 	file_pathStandardize(&workspaceCopy);

// 	if (filePathCopy.length < workspaceCopy.length) {
// 		result = 0;
// 		goto cleanup_l;
// 	}
	
// 	for (divergenceIndex = 0; divergenceIndex < workspaceCopy.length; divergenceIndex++) {
// 		if (filePathCopy.value[divergenceIndex] != workspaceCopy.value[divergenceIndex]) {
// 			result = 0;
// 			goto cleanup_l;
// 		}
// 	}
	
// 	result = 1;
	
// 	cleanup_l:
	
// 	string_free(&filePathCopy);
// 	string_free(&workspaceCopy);
	
// 	return result;
// }
