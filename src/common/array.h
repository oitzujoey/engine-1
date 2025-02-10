#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
#include "allocator.h"

typedef struct {
	void *elements;
	size_t element_size;
	size_t elements_length;
	size_t memory_size;
	Allocator *allocator;
} array_t;

void array_init(array_t *a, Allocator *allocator, size_t element_size);
int array_quit(array_t *a);
size_t array_length(array_t *a);
int array_push(array_t *a, void *element);
int array_getElement(array_t *a, void *element, size_t index);
/* array_pop */
/* array_shrink */

#endif // ARRAY_H
