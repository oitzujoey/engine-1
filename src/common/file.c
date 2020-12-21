
#include "file.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"

char *file_getText(const char *filename) {
	
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

int file_getLine(string_t *line, const char delimiter, FILE *file) {

	char c;
	int error = 0;
	
	line->length = 0;
	line->value[0] = '\0';
	string_realloc_shrink(line);
	
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
