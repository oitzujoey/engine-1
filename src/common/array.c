#include "array.h"
#include <string.h>
#include "common.h"

void array_init(array_t *a, Allocator *allocator, size_t element_size) {
	a->elements = NULL;
	a->element_size = element_size;
	a->elements_length = 0;
	a->memory_size = 0;
	a->allocator = allocator;
}

int array_quit(array_t *a) {
	Allocator *allocator = a->allocator;
	int e = ERR_OK;
	if (a->elements != NULL) {
		e = allocator->free(allocator->context, (void **) &(a->elements));
	}
	a->elements = NULL;
	a->element_size = 0;
	a->elements_length = 0;
	a->memory_size = 0;
	return e;
}

size_t array_length(array_t *a) {
	return a->elements_length;
}

int array_push(array_t *a, void *element) {
	size_t newElements_length = a->elements_length + 1;
	size_t newElements_size = newElements_length * a->element_size;

	// Add space for the new element.
	if (newElements_size > a->memory_size) {
		void *oldMemory = a->elements;
		size_t oldMemory_size = a->memory_size;
		size_t newMemory_size = 2 * newElements_size;
		Allocator *allocator = a->allocator;
		int e = allocator->alloc(allocator->context, &(a->elements), newMemory_size);
		if (e) return e;
		a->memory_size = newMemory_size;
		(void) memcpy(a->elements, oldMemory, oldMemory_size);
		if (oldMemory != NULL) {
			e = allocator->free(allocator->context, &oldMemory);
		}
		if (e) return e;
	}

	void *lastElement = a->elements + a->elements_length * a->element_size;
	if (element == NULL) {
		// Create an empty element.
		(void) memset(lastElement, 0, a->element_size);
	}
	else {
		(void) memcpy(lastElement, element, a->element_size);
	}

	a->elements_length++;

	return ERR_OK;
}

int array_getElement(array_t *a, void *element, size_t index) {
	if (element == NULL) return ERR_NULLPOINTER;

	if (index < a->elements_length) {
		(void) memcpy(element, a->elements + index * a->element_size, a->element_size);
	}
	else {
		return ERR_GENERIC;
	}
	return ERR_OK;
}
