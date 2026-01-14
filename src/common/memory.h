#ifndef MEMORY_H
#define MEMORY_H

void memory_free(void **pointer);
void memory_free_noNullWarning(void **pointer);

#define MEMORY_FREE(pointer) memory_free((void **) (pointer))

#endif // MEMORY_H
