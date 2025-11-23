#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdlib.h>

typedef struct {
	void *context;
	// Malloc
	int (*alloc)(void *allocator, void **data, size_t data_length);
	// Free
	int (*free)(void *allocator, void **data);
	// Quit (for freeing memory in an arena allocator)
	int (*quit)(void *allocator);
} Allocator;

/* Usage:

   Allocator a;
   a.context = myAllocator;
   a.alloc = myAllocator_malloc;
   a.free = myAllocator_free;

   e = a.alloc(a.context, &newDataBlock, newDataBlock_length);
   if (e) panic();
   // Do stuff with `newDataBlock`.
   // ...
   e = a.free(a.context, &newDataBlock);
   if (e) panic(); */

// Wrapper around stdlib's malloc and free.
Allocator allocator_create_stdlib(void);
int allocator_stdlib_malloc(void *allocator, void **data, size_t data_length);
int allocator_stdlib_free(void *allocator, void **data);

// Can only allocate byte lengths of size 0. Can only free null pointers.
Allocator allocator_create_dummy(void);
int allocator_dummy_alloc(void *allocator, void **data, size_t data_length);
int allocator_dummy_free(void *allocator, void **data);
int allocator_dummy_quit(void *allocator);

#endif // ALLOCATOR_H
