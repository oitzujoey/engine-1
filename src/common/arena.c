#include "arena.h"
#include "common.h"

int allocator_create_stdlibArena(Allocator *arena) {
	static Allocator host_allocator = {.context=NULL,
	                                   .alloc=allocator_stdlib_malloc,
	                                   .free=allocator_stdlib_free,
	                                   .quit=allocator_dummy_quit};
	return allocator_create_arena(arena, &host_allocator);
}

int allocator_create_arena(Allocator *arena, Allocator *host_allocator) {
	int e = ERR_OK;

	Arena *internal_arena = NULL;
	e = host_allocator->alloc(host_allocator->context, (void **) &internal_arena, sizeof(Arena));
	if (e) return ERR_OUTOFMEMORY;

	internal_arena->allocations = NULL;
	internal_arena->allocations_length = 0;
	internal_arena->allocations_memoryLength = 0;
	internal_arena->host_allocator = host_allocator;

	arena->context = internal_arena;
	arena->alloc = allocator_arena_alloc;
	arena->free = allocator_arena_free;
	arena->quit = allocator_arena_quit;

	return e;
}

int allocator_arena_alloc(void *allocator, void **data, size_t data_length) {
	int e = ERR_OK;

	// Build namespace
	Arena *arena = allocator;
	Allocator *host = arena->host_allocator;
	void *block = NULL;

	// Allocate
	e = host->alloc(host->context, &block, data_length);
	if (e) return e;

	// Log
	if (arena->allocations_length >= arena->allocations_memoryLength) {
		// Bootstrap multiplication if 0.
		if (arena->allocations_memoryLength == 0) arena->allocations_memoryLength = 1;
		size_t new_memoryLength = 2 * arena->allocations_memoryLength;
		void *new_allocations = NULL;
		e = host->alloc(host->context, &new_allocations, new_memoryLength * sizeof(void *));
		if (e) return e;
		(void) memcpy(new_allocations, arena->allocations, arena->allocations_length * sizeof(void *));
		// Don't free null pointer.
		if (arena->allocations_length) {
			e = host->free(host->context, (void **) &arena->allocations);
		}
		// We may have failed to free the old array, but we can still use the new one.
		arena->allocations = new_allocations;
		arena->allocations_memoryLength = new_memoryLength;
	}
	arena->allocations[arena->allocations_length] = block;
	arena->allocations_length++;

	*data = block;

	return e;
}

int allocator_arena_free(void *allocator, void **data) {
	// Defer all frees until we quit.
	return ERR_OK;
}

int allocator_arena_quit(void *allocator) {
	Arena *arena = allocator;
	Allocator *host = arena->host_allocator;
	size_t allocations_length = arena->allocations_length;
	void **allocations = arena->allocations;
	for (size_t i = 0; i < allocations_length; i++) {
		int e = host->free(host->context, &arena->allocations[i]);
		if (e) return e;
	}
	return host->free(host->context, (void **) &arena);
}
