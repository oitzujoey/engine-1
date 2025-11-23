#ifndef STR4_H
#define STR4_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "allocator.h"
#include "array.h"


/* IMPORTANT:
   STR4 MUST BE USED WITH AN ARENA ALLOCATOR.
   It will call `Allocator:alloc` many times, and then you are expected to call `Allocator:quit`
   once, otherwise none of the memory will be freed. */
/* Important:
   The character at string[string_length] is always '\0'. Binary strings are permitted, so there
   may be null characters at other places in the string as well. */


typedef struct {
	uint8_t *str;  // Str4 functions always add a null terminator.
	size_t str_length;  // Does not include the null added by Str4 functions.
	Allocator *allocator;
	int error;
} Str4;

int str4_errorp(Str4 *str);
Str4 str4_create(Allocator *allocator);
// Never free or reallocate the result of `str4_createConstant`.
Str4 str4_createConstant(const uint8_t *str, size_t str_length);
void str4_copyC(Str4 *str4, const uint8_t *str, size_t str_length);
void str4_copy(Str4 *destination, Str4 *source);
void str4_concatenate(Str4 *destination, Str4 *left, Str4 *right);
void str4_appendC(Str4 *destination, const uint8_t *right, size_t right_length);
void str4_append(Str4 *destination, Str4 *right);
size_t str4_length(Str4 *string);
void str4_substring(Str4 *destination, Str4 *source, ptrdiff_t start_index, ptrdiff_t end_index);
int str4_splitLines(array_t *array, Str4 *string);
bool str4_equalC(Str4 *lstring, const uint8_t *rstring, size_t str_length);

#define CSTR(c_string) ((const uint8_t *) c_string), (sizeof(c_string) - 1)
#define STR4(c_string) str4_createConstant(((const uint8_t *) (c_string)), (sizeof(c_string) - 1))

#endif // STR4_H
