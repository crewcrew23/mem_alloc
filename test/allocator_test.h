#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

#define HEAP_SIZE (1024 * 4) // 4KB 
#define MIN_BLOCK_SIZE 32 //byte

typedef struct header 
{
    size_t size;
    size_t free;
    struct header* next;
} header;



void mem_init(void);
void* mem_alloc(size_t size);
void mem_free(void* ptr);
void* mem_realloc(void* ptr, size_t size);
void* mem_calloc(void* ptr, size_t size);
void alloc_heap_test_(size_t size);


#endif //ALLOCATOR_H