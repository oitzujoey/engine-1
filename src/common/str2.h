
#ifndef STR2_H
#define STR2_H

#include "types.h"

int str2_realloc(char **string, size_t length);
int str2_copy(char *destination, const char *source);
int str2_copyMalloc(char **destination, const char *source);
int str2_copyLengthMalloc(char **destination, const char *source, size_t length);
int str2_concatenateMalloc(char **destination, const char *source);
// void str2_free(str2_t * string);
int str2_removeLineComments(char *line, const char *linecomment);
int str2_removeWhitespace(char *line, const char *config);
// const str2_t *str2_const(const char *chars);
int str2_print(const char *s);
int str2_tokenizeMalloc(char ***tokens, size_t *length, const char *string, const char *delimiters);
void str2_replaceChar(char * const s, const char find, const char replace);

#endif
