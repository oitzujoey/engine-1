
#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include "str.h"

char *file_getText(const char *filename);
int file_getLine(string_t *line, const char delimiter, FILE *file);
/* Returns 1 if file exists. */
int file_exists(const char *filename);
const char *file_getExtension(const char *filename);
int file_isRegularFile(const char *path);
int file_isDirectory(const char *path);

#endif
