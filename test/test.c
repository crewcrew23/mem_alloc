#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "allocator_test.h"

static int tests_run = 0;
static int tests_passed = 0;

#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_RESET "\033[0m"

void ASSERT(int condition, char* test_name, char* message){
    if (!(condition)){
        printf("%s %s: %s %s\n", COLOR_RED, test_name, message, COLOR_RESET);
    }
}

void ASSERT_NULL(void* ptr, char* test_name, char* message){
    if (ptr != NULL){
        printf("%s %s: %s %s\n", COLOR_RED, test_name, message, COLOR_RESET);
    }
}

void ASSERT_NOT_NULL(void* ptr, char* test_name, char* message){
    if (ptr == NULL){
        printf("%s %s: %s %s\n", COLOR_RED, test_name, message, COLOR_RESET);
    }
}

void ASSERT_EQUAL(int v1, int v2, char* test_name, char* message){
    if (v1 != v2){
        printf("%s %s: %s %s\n", COLOR_RED, test_name, message, COLOR_RESET);
    }
}

void ASSERT_EQUAL_ADDR (void* v1, void* v2, char* test_name, char* message){
    if (v1 != v2){
        printf("%s %s: %s %s\n", COLOR_RED, test_name, message, COLOR_RESET);
    }
}

void PASS(char* test_name){
    printf("%s:%s PASS %s\n", test_name, COLOR_GREEN, COLOR_RESET);
}

size_t align(size_t size)
{
    return (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
}

extern header* block_header;
void reset_allocator() {
    block_header = NULL;
}

void test_init(){
    reset_allocator();
    mem_init();
    ASSERT_NOT_NULL(block_header, "test_init", "Initial block should be initialized");
    ASSERT(block_header->size > 0, "test_init", "Initial block should have non-zero size");
    ASSERT_EQUAL(block_header->free, 1, "test_init", "Initial block should be is not free");
    ASSERT_NULL(block_header->next, "test_init", "Initial block next should be is NULL");
    
    PASS("test_init");
}

void test_alloc_zero_size(){
    reset_allocator();
    void* ptr = mem_alloc(0);
    ASSERT_NULL(ptr, "test_alloc_zero_size", "Allocating 0 bytes should return NULL");

    PASS("test_alloc_zero_size");
}

void test_alloc_huge_size(){
    reset_allocator();
    void* ptr = mem_alloc(24000000);
    ASSERT_NOT_NULL(ptr, "test_alloc_huge_size", "Allocating hude size should return valid addr");

    PASS("test_alloc_huge_size");
}

void test_simple_alloc(){
    reset_allocator();
    void* ptr = mem_alloc(100);
    ASSERT_NOT_NULL(ptr, "test_simple_alloc", "Simple allocation should succeed");  
    mem_free(ptr);

    PASS("test_simple_alloc");
}

void test_multiple_allocs(){
    reset_allocator();
    void* ptr1 = mem_alloc(100);
    void* ptr2 = mem_alloc(100);
    void* ptr3 = mem_alloc(100);

    ASSERT_NOT_NULL(ptr1, "test_simple_alloc", "Allocation should succeed");
    ASSERT_NOT_NULL(ptr2, "test_simple_alloc", "Allocation should succeed");
    ASSERT_NOT_NULL(ptr2, "test_simple_alloc", "Allocation should succeed");

    mem_free(ptr1);
    mem_free(ptr2);
    mem_free(ptr3);

    PASS("test_multiple_allocs");
}

void test_free_simple(){
    reset_allocator();
    
    void* ptr = mem_alloc(100);
    ASSERT_NOT_NULL(ptr, "test_free_simple", "Allocation should succeed");

    header* hdr = (header*)((char*)ptr - sizeof(header));
    ASSERT_EQUAL(hdr->free, 0, "test_free_simple", "Block should be marked as not free after allocation");
    
    mem_free(ptr);
    ASSERT_EQUAL(hdr->free, 1, "test_free_simple", "Block should be marked as free after free");

    PASS("test_free_simple");
}

void test_alloc_after_free(){
    reset_allocator();
    
    void* ptr1 = mem_alloc(100);
    ASSERT_NOT_NULL(ptr1, "test_alloc_after_free", "First allocation should succeed");
    
    mem_free(ptr1);
    
    void* ptr2 = mem_alloc(100);
    ASSERT_NOT_NULL(ptr2, "test_alloc_after_free", "Allocation after free should succeed");
    
    ASSERT_EQUAL_ADDR(ptr1, ptr2, "test_alloc_after_free", "Should reuse freed block");

    PASS("test_alloc_after_free");
}

void test_alignment(){
    reset_allocator();

    void* ptr1 = mem_alloc(1);
    void* ptr2 = mem_alloc(5);
    void* ptr3 = mem_alloc(13);
    
    ASSERT_NOT_NULL(ptr1, "test_alignment", "Small allocation should succeed");
    ASSERT_NOT_NULL(ptr2, "test_alignment", "Small allocation should succeed");
    ASSERT_NOT_NULL(ptr3, "test_alignment", "Small allocation should succeed");
    
    ASSERT_EQUAL((uintptr_t)ptr1 % sizeof(void*), 0, "test_alignment", "Pointer should be aligned");
    ASSERT_EQUAL((uintptr_t)ptr2 % sizeof(void*), 0, "test_alignment", "Pointer should be aligned");
    ASSERT_EQUAL((uintptr_t)ptr3 % sizeof(void*), 0, "test_alignment", "Pointer should be aligned");

    PASS("test_alignment");   
}

void test_heap_expansion_allocate_default(){
    reset_allocator();
    alloc_heap_test_(32);
    
    ASSERT_NOT_NULL(block_header, "test_heap_expansion_allocate_default", "block_heder should be not NULL");
    
    void* ptr = mem_alloc(100);
     
    ASSERT_NOT_NULL(ptr, "test_heap_expansion_allocate_default", "new alloc block should be not NULL");

    mem_free(ptr);
    PASS("test_heap_expansion_allocate_default"); 
}

void test_fast_forward_coalescing_asc(){
    reset_allocator();
    
        void* ptr1 = mem_alloc(1);     
    void* ptr2 = mem_alloc(3);     
    void* ptr3 = mem_alloc(13);     
    void* ptr4 = mem_alloc(23);     
    void* ptr5 = mem_alloc(31);

    mem_free(ptr1);
    mem_free(ptr2);
    mem_free(ptr3);
    mem_free(ptr4);
    mem_free(ptr5);

    PASS("test_fast_forward_coalescing_asc"); 
}

void test_fast_forward_coalescing_desc(){
    reset_allocator();
    
    void* ptr1 = mem_alloc(1);     
    void* ptr2 = mem_alloc(3);     
    void* ptr3 = mem_alloc(13);     
    void* ptr4 = mem_alloc(23);     
    void* ptr5 = mem_alloc(31);
    
    mem_free(ptr5);
    mem_free(ptr4);
    mem_free(ptr3);
    mem_free(ptr2);
    mem_free(ptr1);

    PASS("test_fast_forward_coalescing_desc"); 
}

void test_simple_realoc(){
    reset_allocator();

    int* ptr = mem_alloc(3 * sizeof(int));     

    for (size_t i = 0; i < 3; ++i)
    {
        *(ptr+i) = 1;
    }

    ptr = mem_realloc(ptr, 4 * sizeof(int));     

    for (size_t i = 0; i < 3; ++i)
    {
        ASSERT((*(ptr+i) == 1), "test_simple_realoc", "invalid copy value");
    }

    mem_free(ptr);
    PASS("test_simple_realoc"); 
}

void test_realoc_null_value(){
    reset_allocator();

    int* ptr = mem_alloc(3 * sizeof(int));     
    ptr = mem_realloc(NULL, 4 * sizeof(int));
    ASSERT_NULL(ptr, "test_realoc_null_value", "value must be null");     

    mem_free(ptr);
    PASS("test_realoc_null_value"); 
}

void test_realoc_zero_value(){
    reset_allocator();

    int* ptr = mem_alloc(3 * sizeof(int));     
    ptr = mem_realloc(ptr, 0);
    ASSERT_NULL(ptr, "test_realoc_zero_value", "value must be null");     

    mem_free(ptr);
    PASS("test_realoc_zero_value"); 
}

void test_simpe_calloc(){
    reset_allocator();

    int* ptr = mem_calloc(3, sizeof(int));     
    size_t align_size = align(3 * sizeof(int));
    ASSERT((((header *)((char *)ptr - sizeof(header)))->size == align_size), "test_simpe_calloc", "invalid size of alloc block");   

    mem_free(ptr);
    PASS("test_simpe_calloc"); 
}

void main(){
    test_init();
    test_alloc_zero_size();
    test_alloc_huge_size();
    test_simple_alloc();
    test_multiple_allocs();
    test_free_simple();
    test_alloc_after_free();
    test_alignment();
    test_heap_expansion_allocate_default();
    test_fast_forward_coalescing_asc();
    test_fast_forward_coalescing_desc();
    test_simple_realoc();
    test_realoc_null_value();
    test_realoc_zero_value();
    test_simpe_calloc();
}