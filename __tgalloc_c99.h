#ifndef TGALLOC_C99_H
#define TGALLOC_C99_H

// Helper macros for argument counting and selection in variadic macros
#define _tgalloc_palloc_arg3(x1, x2, x3, ...) x3
#define _tgalloc_palloc_choose(...) \
    _tgalloc_palloc_arg3(__VA_ARGS__, _tgalloc_palloc2, _tgalloc_palloc1)

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
#define palloc(...) _tgalloc_palloc_choose(__VA_ARGS__)(__VA_ARGS__)

// Helper macro for choosing between single and array free functions
#define _tgalloc_pfree_choose(...) _tgalloc_palloc_arg3(__VA_ARGS__, _tgalloc_pfree2, _tgalloc_pfree1)

/**
 * @brief Free memory allocated with palloc
 * 
 * Deallocates memory previously allocated with palloc. For arrays, provide the 
 * same length parameter that was used during allocation.
 * 
 * @param ptr Pointer to the memory to free
 * @param ... Optional length parameter for array allocations
 */
#define pfree(...) _tgalloc_pfree_choose(__VA_ARGS__)(__VA_ARGS__)


#endif // TGALLOC_C99_H
