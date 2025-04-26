/**
 * @file tgalloc_tests.c
 * @brief Test suite for the tgalloc library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "../tgalloc.h"

// Custom testing macros
#define TEST_CASE(name) void test_##name(void)
#define RUN_TEST(name) do { \
    printf("Running test: %s... ", #name); \
    test_##name(); \
    printf("PASSED\n"); \
} while (0)

// Some custom structures to test with
typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    double values[16];  // 128 bytes with alignment requirements
} AlignedStruct;

// Custom allocator for testing
typedef struct {
    size_t alloc_count;
    size_t free_count;
    size_t total_allocated;
    bool should_fail;
} TestAllocator;

TestAllocator test_allocator = {0};

void* test_alloc(TestAllocator* self, size_t alignment, size_t size) {
    (void)alignment; // Mark as unused to prevent compiler warnings
    
    if (self->should_fail) return NULL;
    
    self->alloc_count++;
    self->total_allocated += size;
    
    // Simplified alignment handling for test purposes
    void* ptr = malloc(size);
    return ptr;
}

void* test_realloc(TestAllocator* self, void* ptr, size_t alignment, 
                  size_t old_size, size_t new_size) {
    (void)alignment; // Mark as unused to prevent compiler warnings
    
    if (self->should_fail) return NULL;
    
    self->total_allocated = self->total_allocated - old_size + new_size;
    return realloc(ptr, new_size);
}

void test_free(TestAllocator* self, void const* ptr, size_t alignment, size_t size) {
    (void)alignment; // Mark as unused to prevent compiler warnings
    
    if (ptr == NULL) return;
    
    self->free_count++;
    self->total_allocated -= size;
    free((void*)ptr);
}

void reset_test_allocator() {
    test_allocator.alloc_count = 0;
    test_allocator.free_count = 0;
    test_allocator.total_allocated = 0;
    test_allocator.should_fail = false;
}

// Test cases

TEST_CASE(single_alloc_free) {
    int* num = NULL;
    ppalloc(&num);
    
    assert(num != NULL);
    *num = 42;
    assert(*num == 42);
    
    pfree(num);
}

TEST_CASE(array_alloc_free) {
    const size_t array_size = 10;
    int* nums = NULL;
    ppalloc(&nums, array_size);
    
    assert(nums != NULL);
    
    for (size_t i = 0; i < array_size; i++) {
        nums[i] = (int)i * 10;
    }
    
    for (size_t i = 0; i < array_size; i++) {
        assert(nums[i] == (int)i * 10);
    }
    
    pfree(nums, array_size);
}

TEST_CASE(struct_alloc_free) {
    Point* point = NULL;
    ppalloc(&point);
    
    assert(point != NULL);
    
    point->x = 10;
    point->y = 20;
    
    assert(point->x == 10);
    assert(point->y == 20);
    
    pfree(point);
}

TEST_CASE(array_realloc) {
    const size_t initial_size = 5;
    const size_t new_size = 10;
    
    int* nums = NULL;
    ppalloc(&nums, initial_size);
    
    assert(nums != NULL);
    
    for (size_t i = 0; i < initial_size; i++) {
        nums[i] = (int)i;
    }
    
    pprealloc(&nums, initial_size, new_size);
    
    assert(nums != NULL);
    
    // Check that original values are preserved
    for (size_t i = 0; i < initial_size; i++) {
        assert(nums[i] == (int)i);
    }
    
    // Set and check new values
    for (size_t i = initial_size; i < new_size; i++) {
        nums[i] = (int)i * 2;
    }
    
    for (size_t i = initial_size; i < new_size; i++) {
        assert(nums[i] == (int)i * 2);
    }
    
    pfree(nums, new_size);
}

TEST_CASE(custom_allocator) {
    reset_test_allocator();
    
    // Switch to custom allocator
    #undef TGA
    #undef TGA_ALLOC_A_S
    #undef TGA_REALLOC_A_S
    #undef TGA_FREE_A_S
    
    #define TGA (TestAllocator*)&test_allocator
    #define TGA_ALLOC_A_S test_alloc
    #define TGA_REALLOC_A_S test_realloc
    #define TGA_FREE_A_S test_free
    
    // Use the custom allocator
    int* nums = NULL;
    ppalloc(&nums, 10);
    
    assert(nums != NULL);
    assert(test_allocator.alloc_count == 1);
    assert(test_allocator.total_allocated == 10 * sizeof(int));
    
    for (int i = 0; i < 10; i++) {
        nums[i] = i;
    }
    
    pfree(nums, 10);
    
    assert(test_allocator.free_count == 1);
    assert(test_allocator.total_allocated == 0);
    
    // Test reallocation
    double* doubles = NULL;
    ppalloc(&doubles, 5);
    
    assert(doubles != NULL);
    assert(test_allocator.alloc_count == 2);
    
    for (int i = 0; i < 5; i++) {
        doubles[i] = i * 1.5;
    }
    
    size_t before_realloc = test_allocator.total_allocated;
    pprealloc(&doubles, 5, 8);
    
    assert(doubles != NULL);
    assert(test_allocator.total_allocated == before_realloc - 5 * sizeof(double) + 8 * sizeof(double));
    
    pfree(doubles, 8);
    assert(test_allocator.free_count == 2);
    assert(test_allocator.total_allocated == 0);
    
    // Reset to default allocator
    #undef TGA
    #undef TGA_ALLOC_A_S
    #undef TGA_REALLOC_A_S
    #undef TGA_FREE_A_S
    
    #define TGA (dflt_allo_t*)NULL
    #define TGA_ALLOC_A_S _tgalloc_dflt_alloc_aligned_sized
    #define TGA_REALLOC_A_S _tgalloc_dflt_realloc_aligned_sized
    #define TGA_FREE_A_S _tgalloc_dflt_free_aligned_sized
}

TEST_CASE(allocation_failure) {
    reset_test_allocator();
    test_allocator.should_fail = true;
    
    // Switch to custom allocator
    #undef TGA
    #undef TGA_ALLOC_A_S
    #undef TGA_REALLOC_A_S
    #undef TGA_FREE_A_S
    
    #define TGA (TestAllocator*)&test_allocator
    #define TGA_ALLOC_A_S test_alloc
    #define TGA_REALLOC_A_S test_realloc
    #define TGA_FREE_A_S test_free
    
    // This allocation should fail
    int* nums = NULL;
    ppalloc(&nums, 10);
    
    assert(nums == NULL);
    
    // Reset to default allocator
    #undef TGA
    #undef TGA_ALLOC_A_S
    #undef TGA_REALLOC_A_S
    #undef TGA_FREE_A_S
    
    #define TGA (dflt_allo_t*)NULL
    #define TGA_ALLOC_A_S _tgalloc_dflt_alloc_aligned_sized
    #define TGA_REALLOC_A_S _tgalloc_dflt_realloc_aligned_sized
    #define TGA_FREE_A_S _tgalloc_dflt_free_aligned_sized
}

TEST_CASE(complex_struct_array) {
    const size_t num_objects = 5;
    
    // Allocate an array of complex structs
    AlignedStruct* structs = NULL;
    ppalloc(&structs, num_objects);
    
    assert(structs != NULL);
    
    // Initialize with data
    for (size_t i = 0; i < num_objects; i++) {
        for (size_t j = 0; j < 16; j++) {
            structs[i].values[j] = i * 100.0 + j;
        }
    }
    
    // Verify data
    for (size_t i = 0; i < num_objects; i++) {
        for (size_t j = 0; j < 16; j++) {
            assert(structs[i].values[j] == i * 100.0 + j);
        }
    }
    
    pfree(structs, num_objects);
}

int main() {
    printf("Running tgalloc tests...\n");
    
    RUN_TEST(single_alloc_free);
    RUN_TEST(array_alloc_free);
    RUN_TEST(struct_alloc_free);
    RUN_TEST(array_realloc);
    RUN_TEST(custom_allocator);
    RUN_TEST(allocation_failure);
    RUN_TEST(complex_struct_array);
    
    printf("All tests passed successfully!\n");
    return 0;
}
