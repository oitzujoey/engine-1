#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>
#include "allocator.h"

typedef struct {
	Allocator *host_allocator;
	void **allocations;
	size_t allocations_length;
	size_t allocations_memoryLength;
} Arena;

int allocator_create_stdlibArena(Allocator *arena);
int allocator_create_arena(Allocator *arena, Allocator *host_allocator);
int allocator_arena_alloc(void *allocator, void **data, size_t data_length);
int allocator_arena_free(void *allocator, void **data);
int allocator_arena_quit(void *allocator);

#define ARENA_ALLOC(arena, data, data_length) arena.alloc(arena.context, (void **) data, data_length)

#endif // ARENA_H
