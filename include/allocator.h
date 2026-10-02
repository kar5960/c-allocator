#include <stddef.h>
#ifndef ALLOCATOR_H
#define ALLOCATOR_H

void * my_malloc(size_t size);
void my_free(void *ptr);

#endif