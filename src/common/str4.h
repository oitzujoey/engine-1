#ifndef STR4_H
#define STR4_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "allocator.h"


/* IMPORTANT:
   STR4 MUST BE USED WITH AN ARENA ALLOCATOR.
   It will call `Allocator:alloc` many times, and then you are expected to call `Allocator:quit` once. */


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

#define STR4(c_string) str4_createConstant(((const uint8_t *) (c_string)), (sizeof(c_string) - 1))

#endif // STR4_H
