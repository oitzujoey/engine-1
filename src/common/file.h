
#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include "types.h"

// char *file_getText(const char *filename);
// int file_getLine(char *line, const char delimiter, FILE *file);
/* Returns 1 if file exists. */
// int file_exists(const char *filename);
const char *file_getExtension(const char *filename);
// void file_isRegularFile(const char *path);
// void file_isDirectory(const char *path);
int file_concatenatePath(char **destination, const char *source);
void file_resolveRelativePaths(char *path);
// int file_pathStandardize(char *filePath);
// int file_pathIsInDirectory(const char *filePath, const char* workspace);

int file_parse_uint8(uint8_t *v, uint8_t *bytes, size_t *index, size_t length);
int file_parse_uint32(uint32_t *v, uint8_t *bytes, size_t *index, size_t length);
int file_parse_float(float *v, uint8_t *bytes, size_t *index, size_t length);
int file_parse_double(double *v, uint8_t *bytes, size_t *index, size_t length);
int file_parse_vec(vec_t *v, uint8_t *bytes, size_t *index, size_t length);
int file_parse_vecArray(vec_t *v, size_t v_length, uint8_t *bytes, size_t *index, size_t length);
int file_parse_vec3(vec3_t v, uint8_t *bytes, size_t *index, size_t length);

#endif
