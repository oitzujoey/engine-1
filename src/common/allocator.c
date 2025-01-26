#include "allocator.h"
#include "memory.h"
#include "common.h"
#include "log.h"

int allocator_stdlib_malloc(void *allocator, void **data, size_t data_length) {
	if (!data_length) {
		*data = NULL;
		warning("allocator_stdlib_malloc", "Allocated a block of length zero.");
		return ERR_OK;
	}
	*data = malloc(data_length);
	if (!*data) return ERR_OUTOFMEMORY;
	return ERR_OK;
}

int allocator_stdlib_free(void *allocator, void **data) {
	memory_free(data);
	return ERR_OK;
}

Allocator allocator_create_stdlib(void) {
	return (Allocator) {.context=NULL, .alloc=allocator_stdlib_malloc, .free=allocator_stdlib_free, .quit=allocator_dummy_quit};
}


int allocator_dummy_alloc(void *a, void **d, size_t l) {
	if (l) {
		warning("allocator_dummy_alloc", "Attempted to allocate using dummy allocator.");
		return ERR_OUTOFMEMORY;
	}
	// Allocating 0 bytes is fine.
	*d = NULL;
	return ERR_OK;
}

int allocator_dummy_free(void *a, void **d) {
	if (*d) {
		warning("allocator_dummy_free", "Attempted to free using dummy allocator.");
		return ERR_OUTOFMEMORY;
	}
	// Freeing a NULL pointer is fine.
	return ERR_OK;
}

int allocator_dummy_quit(void *a) {
	// I guess quitting is always fine.
	return ERR_OK;
}

Allocator allocator_create_dummy(void) {
	return (Allocator) {.context=NULL, .alloc=allocator_dummy_alloc, .free=allocator_dummy_free, .quit=allocator_dummy_quit};
}
