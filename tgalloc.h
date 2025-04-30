//
// Created by Jethro Su on 22/12/2023.
//

#ifndef TGALLOC_H
#define TGALLOC_H

// #define __TGALLOC_DISPLAY_C_VERSION__

/**
 * @file tgalloc.h
 * @brief A type-generic memory allocation library for C
 * 
 * This library provides macros for type-safe memory allocation, reallocation,
 * and deallocation that respect alignment requirements of the allocated types.
 * It uses the C preprocessor to generate appropriate function calls with type
 * information derived from the variables being allocated.
 */

#include <stdlib.h>

/**
 * @typedef dflt_allo_t
 * @brief Default allocator type
 * 
 * Structure representing the default allocator. When using the default allocator,
 * this can be NULL as the actual implementation uses standard malloc/realloc/free.
 */
typedef struct dflt_allo_t dflt_allo_t;

// Default allocation function using malloc
static void * _tgalloc_dflt_alloc_aligned_sized(dflt_allo_t * self, size_t alignment, size_t size){
    (void)self;      // Mark as unused to prevent compiler warnings
    (void)alignment; // Mark as unused to prevent compiler warnings
    return malloc(size);
}

// Default reallocation function using realloc
static void * _tgalloc_dflt_realloc_aligned_sized(dflt_allo_t * self, void * ptr, size_t alignment, size_t old_size, size_t new_size){
    (void)self;      // Mark as unused to prevent compiler warnings
    (void)alignment; // Mark as unused to prevent compiler warnings
    (void)old_size;  // Mark as unused to prevent compiler warnings
    return realloc(ptr, new_size);
}

// Default deallocation function using free
static void _tgalloc_dflt_free_aligned_sized(dflt_allo_t * self, void const * ptr,
                                        size_t alignment, size_t size){
    (void)self;      // Mark as unused to prevent compiler warnings
    (void)alignment; // Mark as unused to prevent compiler warnings
    (void)size;      // Mark as unused to prevent compiler warnings
    free((void*)ptr);
    
}

/**
 * @def TGA
 * @brief Default allocator instance
 * 
 * NULL pointer cast to dflt_allo_t* for use with the default allocation functions.
 * 
 * To use a custom allocator, define TGA before including this header:
 * #define TGA (your_allocator_t*)&your_allocator_instance
 * 
 * You must also provide the corresponding allocation function implementations
 * through TGA_ALLOC_A_S, TGA_REALLOC_A_S, and TGA_FREE_A_S.
 */

/**
 * @def TGA_ALLOC_A_S
 * @brief Allocation function pointer
 * 
 * Points to a function that allocates memory with alignment and size requirements.
 * The function must have the signature:
 * void* function(allocator_t* self, size_t alignment, size_t size)
 * 
 * Parameters:
 * - self: Pointer to the allocator instance
 * - alignment: Required alignment for the allocated memory (in bytes)
 * - size: Required size for the allocated memory (in bytes)
 * 
 * Return value:
 * - Pointer to the allocated memory, or NULL on failure
 */

/**
 * @def TGA_REALLOC_A_S
 * @brief Reallocation function pointer
 * 
 * Points to a function that reallocates memory with alignment and size requirements.
 * The function must have the signature:
 * void* function(allocator_t* self, void* ptr, size_t alignment, size_t old_size, size_t new_size)
 * 
 * Parameters:
 * - self: Pointer to the allocator instance
 * - ptr: Pointer to the memory to reallocate
 * - alignment: Required alignment for the memory (in bytes)
 * - old_size: Current size of the memory block (in bytes)
 * - new_size: New size for the memory block (in bytes)
 * 
 * Return value:
 * - Pointer to the reallocated memory, or NULL on failure
 */

/**
 * @def TGA_FREE_A_S
 * @brief Deallocation function pointer
 * 
 * Points to a function that frees memory.
 * The function must have the signature:
 * void function(allocator_t* self, void const* ptr, size_t alignment, size_t size)
 * 
 * Parameters:
 * - self: Pointer to the allocator instance
 * - ptr: Pointer to the memory to free
 * - alignment: Alignment of the memory (in bytes)
 * - size: Size of the memory block (in bytes)
 */
#ifndef TGA
#define TGA (dflt_allo_t*)NULL
#define TGA_ALLOC_A_S _tgalloc_dflt_alloc_aligned_sized
#define TGA_REALLOC_A_S _tgalloc_dflt_realloc_aligned_sized
#define TGA_FREE_A_S _tgalloc_dflt_free_aligned_sized
#endif // TGA

// Utility macros to get alignment and size information

// Check for typeof support across different compilers
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000L
   // C23 has native typeof
   #define _tgalloc_typeof(x) typeof(x)
#elif defined(__GNUC__) || defined(__clang__)
   // GCC and Clang support __typeof__
   // Enable alignof support in C11 mode
   #include <stdalign.h>
   #define _tgalloc_typeof(x) __typeof__(x)
#elif defined(_MSC_VER) && defined(_MSVC_LANG) && _MSVC_LANG >= 202000L
   // MSVC with C++23 mode might have typeof support
   #define _tgalloc_typeof(x) typeof(x)
#else
   // Fallback for compilers without typeof support
   // WARNING: This removes type safety and only works with pointers
   #define _tgalloc_typeof(x) void*
   #warning "typeof not available: type safety compromised, allocator only safe for pointer types"
#endif

// Check for alignof support
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
   // C11 has _Alignof (alignof from stdalign.h)
   #define _tgalloc_alignof_impl(T) alignof(T)
#elif defined(__GNUC__) || defined(__clang__)
   // GCC and Clang support __alignof__
   #define _tgalloc_alignof_impl(T) __alignof__(T)
#else
   // Fallback for compilers without alignof support
   // A common alignment strategy is to use the size for fundamental types
   // or a default conservative alignment (using sizeof(void*) instead of 8)
   #define _tgalloc_alignof_impl(T) ((sizeof(T) < sizeof(void*)) ? sizeof(T) : sizeof(void*))
   #warning "alignof not available: using size-based alignment fallback"
#endif

// Define the actual macros using our conditional implementations
#define _tgalloc_alignof(ptr) _tgalloc_alignof_impl(_tgalloc_typeof(*(ptr)))
#define _tgalloc_sizeof(ptr) sizeof(*(ptr))
#define _tgalloc_sizeof_n(ptr, n) ((_tgalloc_sizeof(ptr)) * (n))

// Helper macro for allocating memory for a single object
#define _tgalloc_palloc1(ptr) \
    ((ptr) = (_tgalloc_typeof(ptr))TGA_ALLOC_A_S(TGA, _tgalloc_alignof(ptr), _tgalloc_sizeof(ptr)))

// Helper macro for allocating memory for an array of objects
#define _tgalloc_palloc2(ptr, len) \
    ((ptr) = (_tgalloc_typeof(ptr))TGA_ALLOC_A_S(TGA, _tgalloc_alignof(ptr), _tgalloc_sizeof_n(ptr, len)))

// Helper macro for freeing memory of a single object
#define _tgalloc_pfree1(ptr) TGA_FREE_A_S(TGA, (void*)(ptr), _tgalloc_alignof(ptr), _tgalloc_sizeof(ptr))

// Helper macro for freeing memory of an array of objects
#define _tgalloc_pfree2(ptr, len) TGA_FREE_A_S(TGA, (void*)(ptr), _tgalloc_alignof(ptr), _tgalloc_sizeof_n(ptr, len))

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000L
    #include "__tgalloc_c23.h"
#else
    #include "__tgalloc_c99.h"
#endif

/**
 * @brief Allocate memory through a pointer to pointer
 * 
 * Indirection helper that allows allocating memory when you have a pointer
 * to a pointer. Useful for functions that need to allocate and return memory.
 * 
 * @param pptr Pointer to pointer that will receive the allocated memory
 * @param ... Optional length parameter for array allocations
 */

#define ppalloc(pptr, ...) palloc(*(pptr) __VA_OPT__(,) __VA_ARGS__)

 // Uncomment to enable ppfree functionality
 // /**
 //  * @brief Free memory allocated with ppalloc
 //  * 
 //  * Indirection helper that allows freeing memory allocated with ppalloc
 //  * through a pointer to pointer.
 //  * 
 //  * @param pptr Pointer to pointer to the memory to free
 //  * @param ... Optional length parameter for array allocations
 //  */
 // #define ppfree(pptr, ...) pfree(*(pptr) __VA_OPT__(,) __VA_ARGS__)
 
 // Helper macro for reallocating memory
#define _tgalloc_realloc2(ptr, old_len, new_len) TGA_REALLOC_A_S(TGA, (void*)(ptr), _tgalloc_alignof(ptr), \
     _tgalloc_sizeof_n(ptr, old_len), _tgalloc_sizeof_n(ptr, new_len))
 
 /**
  * @brief Reallocate memory through a pointer to pointer
  * 
  * Resizes a previously allocated array to a new size while preserving its contents.
  * If new_len is larger than old_len, the additional elements will be uninitialized.
  * If new_len is smaller, the excess elements will be lost.
  * 
  * @param pptr    Pointer to pointer to the memory to reallocate
  * @param old_len Current number of elements
  * @param new_len Desired number of elements after reallocation
  * @return The assigned pointer (for chaining operations)
  */
#define pprealloc(pptr, old_len, new_len) (*(pptr)=_tgalloc_realloc2(*(pptr), old_len, new_len))

#endif // TGALLOC_H
