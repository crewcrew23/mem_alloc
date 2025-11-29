#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

#define HEAP_SIZE (4 * 1024) // 4KB
#define MIN_BLOCK_SIZE 32 //byte

typedef struct header 
{
    size_t size;
    size_t free;
    struct header* next;
} header;

/** @file allocator.h
 */
/**
 * \mainpage dox mem_alloc
 
 *
 * - \ref allocator.h
 */


/**
 * @brief allocate a heap of default size 4KB.
 *
 *
 */
void mem_init(void);

/**
 * @brief alloc space by N size
 *
 *
 * @param size size of bytes.
 * @return space as void pointer.
 */
void* mem_alloc(size_t size);

/**
 * @brief free up space after allocate.
 *
 * @param ptr pointer to the data to be free.
 */
void mem_free(void* ptr);

/**
 * @brief extend allocated space.
 *
 * @param ptr pointer to the data to be extended.
 * @param size size of bytes a new space.
 * @return new extended space
 */
void* mem_realloc(void* ptr, size_t size);

/**
 * @brief in dev.
 */
void* mem_calloc(void* ptr, size_t size);


#endif //ALLOCATOR_H