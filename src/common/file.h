
#ifndef FILE_H
#define FILE_H

#include <stdio.h>

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

#endif
