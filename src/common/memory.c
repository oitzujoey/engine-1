#include "memory.h"
#include "log.h"
#include <stdlib.h>

void memory_free(void **pointer) {
	if (*pointer) {
		free(*pointer);
		*pointer = NULL;
	}
	else {
		log_warning("memory_free", "Attempted to free a null pointer.");
	}
}

void memory_free_noNullWarning(void **pointer) {
	if (*pointer) {
		free(*pointer);
		*pointer = NULL;
	}
}
