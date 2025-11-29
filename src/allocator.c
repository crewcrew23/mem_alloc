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

void *allocateN(size_t size)
{
    return mmap(NULL, size,
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
    int remaining = block->size - size - sizeof(header);
    if (remaining < MIN_BLOCK_SIZE)
        return;

    header *new_block = (header *)((char *)block + sizeof(header) + size);
    new_block->size = remaining;
    new_block->free = 1;
    new_block->next = block->next;

    block->size = size;
    block->next = new_block;
}


void *extend_heap(size_t size)
{
    void *heap;
    size_t heap_size = HEAP_SIZE;

    if (size >= HEAP_SIZE)
    {
        heap = allocateN(size);
        heap_size = size;
    }
    else
    {
        heap = allocate_default();
    }

    if (heap == MAP_FAILED)
        return NULL;

    header *new_block = (header *)heap;
    new_block->free = 1;
    new_block->size = heap_size - sizeof(header);

    header *current = block_header;
    while (current != NULL)
    {
        if (current->next == NULL)
        {
            current->next = new_block;
            return (void *)new_block;
        }
        current = current->next;
    }

    return NULL;
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
        void *new_space = extend_heap(size);
        block = (header*)new_space;
    }

    split_block(block, size);
    block->free = 0;

    return (void *)((char *)block + sizeof(header));
}

void fast_forward_coalescing()
{
    header *current = block_header;
    while (current != NULL)
    {
        if (current->next == NULL)
            break;

        if (current->free & current->next->free)
        {
            current->size += current->next->size;
            current->next = current->next->next;
        }
        current = current->next;
    }
}

void mem_free(void *ptr)
{
    if (ptr != NULL)
    {
        ((header *)((char *)ptr - sizeof(header)))->free = 1;
        fast_forward_coalescing();
    }
}

size_t union_next(void *ptr)
{
    header *current = (header *)((char *)ptr - sizeof(header));
    if (current->next == NULL)
        return 1;

    if (current->next->free)
    {
        current->size += current->next->size;
        current->next = current->next->next;
        return current->size;
    }

    return 1;
}


void mem_cpy(void* dst, void* src, size_t size) {
    char* src_c = (char*)src;
    char* dst_c = (char*)dst;
    
    for (size_t i = 0; i < size; i++) {
        *dst_c++ = *src_c++;
    }
}

void *mem_realloc(void *ptr, size_t size)
{
    if (ptr == NULL)
        return NULL;

    if (size == 0){
        ((header *)((char *)ptr - sizeof(header)))->free = 1;
        return NULL;
    }
    
    size = align(size);

    header *block = find_free_block(size);
    if (block == NULL)
    {
        size_t sz = union_next(ptr);
        if (sz >= size)
        {
            split_block((header *)ptr, sz);
            return (void *)((char *)ptr + sizeof(header));
        }

        fast_forward_coalescing();
        block = find_free_block(size);
        if (block == NULL)
        {
            size += 32;
            void *new_block = extend_heap(size);
            if (new_block == NULL)
                return NULL;

            header* ret_block = (header*)new_block;
            split_block(ret_block, size);
            ((header*)((char*)ptr - sizeof(header)))->free = 1;
            mem_cpy((void *)((char *)ret_block + sizeof(header)), ptr, ((header*)((char*)ptr - sizeof(header)))->size);
            return (void *)((char *)ret_block + sizeof(header));
        }
        split_block(block, size);
        ((header*)((char*)ptr - sizeof(header)))->free = 1;
        mem_cpy((void *)((char *)block + sizeof(header)), ptr, ((header*)((char*)ptr - sizeof(header)))->size);
        return (void *)((char *)block + sizeof(header));
    }

    split_block(block, size);
    block->free = 0;
    ((header *)ptr)->free = 1;
    mem_cpy((void *)((char *)block + sizeof(header)), ptr, ((header*)((char*)ptr - sizeof(header)))->size);
    return (void *)((char *)block + sizeof(header));
}

void *mem_calloc(void *ptr, size_t size) {}