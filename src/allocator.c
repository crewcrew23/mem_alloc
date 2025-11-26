#include <sys/mman.h>
#include <stdio.h>

#include "allocator.h"

header *block_header = NULL;

static size_t align(size_t size)
{
    return (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
}

void *allocate_default()
{
    return mmap(NULL, HEAP_SIZE,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void alloc_heap_test_(size_t size)
{
    void *heap = mmap(NULL, size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (heap == MAP_FAILED)
    {
        perror("mmap failed");
        return;
    }

    block_header = (header *)heap;
    block_header->size = size - sizeof(header);
    block_header->free = 1;
    block_header->next = NULL;
}

void mem_init(void)
{
    void *heap = allocate_default();

    if (heap == MAP_FAILED)
    {
        perror("mmap failed");
        return;
    }

    block_header = (header *)heap;
    block_header->size = HEAP_SIZE - sizeof(header);
    block_header->free = 1;
    block_header->next = NULL;
}

void *find_free_block(size_t size)
{
    header *current = block_header;
    while (current != NULL)
    {
        if (current->free && current->size >= size)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

static void split_block(header *block, size_t size)
{
    size_t remaining = block->size - size - sizeof(header);
    if (remaining < MIN_BLOCK_SIZE)
        return;

    header *new_block = (header *)((char *)block + sizeof(header) + size);
    new_block->size = remaining;
    new_block->free = 1;
    new_block->next = block->next;

    block->size = size;
    block->next = new_block;
}

void *mem_alloc(size_t size)
{
    if (size == 0)
        return NULL;
    if (block_header == NULL)
        mem_init();

    size = align(size);

    header *block = find_free_block(size);
    if (block == NULL)
    {
        void *new_space = allocate_default();

        if (new_space == MAP_FAILED)
        {
            perror("mmap failed");
            return NULL;
        }

        header *new_block = (header *)new_space;
        new_block->size = HEAP_SIZE - sizeof(header);
        new_block->free = 1;
        new_block->next = NULL;

        header *current = block_header;
        while (current->next != NULL)
            current = current->next;

        current->next = new_block;
        block = new_block;
    }

    split_block(block, size);
    block->free = 0;

    return (void *)((char *)block + sizeof(header));
}

void mem_free(void *ptr)
{
    if (ptr != NULL)
    {
        // TODO: merge with prevous block
        ((header *)((char *)ptr - sizeof(header)))->free = 1;
        ptr = NULL;
    }
}
void *mem_realloc(void *ptr, size_t size) {}
void *mem_calloc(void *ptr, size_t size) {}
