//
// Created by Jethro Su on 22/12/2023.
//

#ifndef TGALLOC_C23_H
#define TGALLOC_C23_H

#ifdef __TGALLOC_DISPLAY_C_VERSION__
    #warning "C23 support detected"
#endif

// Helper macros for argument counting and selection in variadic macros
#define _tgalloc_palloc_arg2(x1, x2, ...) x2
#define _tgalloc_palloc_choose(...) \
    _tgalloc_palloc_arg2(__VA_ARGS__ __VA_OPT__(,) _tgalloc_palloc2, _tgalloc_palloc1)

/**
 * @brief Allocate memory for an object or array of objects
 * 
 * Allocates memory for a single object or an array of objects, automatically deriving
 * the type information from the pointer. For arrays, provide the length as the second parameter.
 * 
 * @param ptr Pointer that will receive the allocated memory
 * @param ... Optional length parameter for array allocations
 * @return The assigned pointer (for chaining operations)
 */
#define palloc(ptr, ...) _tgalloc_palloc_choose(__VA_ARGS__)(ptr __VA_OPT__(,) __VA_ARGS__)

// Helper macro for choosing between single and array free functions
#define _tgalloc_pfree_choose(...) _tgalloc_palloc_arg2(__VA_ARGS__ __VA_OPT__(,) _tgalloc_pfree2, _tgalloc_pfree1)

/**
 * @brief Free memory allocated with palloc
 * 
 * Deallocates memory previously allocated with palloc. For arrays, provide the 
 * same length parameter that was used during allocation.
 * 
 * @param ptr Pointer to the memory to free
 * @param ... Optional length parameter for array allocations
 */
#define pfree(ptr, ...) _tgalloc_pfree_choose(__VA_ARGS__)(ptr __VA_OPT__(,) __VA_ARGS__)

#endif // TGALLOC_C23_H
