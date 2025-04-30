# tgalloc - Type-Generic Memory Allocator for C

A header-only library that provides type-safe memory allocation, reallocation, and deallocation macros for C with automatic type inference and alignment handling.

## Features

- **Type-safe allocation**: Automatically infers type information from pointers
- **Alignment-aware**: Respects alignment requirements of allocated types
- **Single and array allocations**: Unified interface for both single objects and arrays
- **Custom allocator support**: Use your own allocator implementation
- **Minimal dependencies**: Only requires C99 standard library

## Installation

Simply include the header file in your project:

```c
#include "tgalloc.h"
```

## Usage

### Basic Allocation and Deallocation

```c
// Allocate a single integer
int* num = NULL;
ppalloc(&num);

// Use the allocated memory
*num = 42;

// Free the memory when done
pfree(num);

// Allocate an array of 10 integers
int* nums = NULL;
ppalloc(&nums, 10);

// Use the array
for (int i = 0; i < 10; i++) {
    nums[i] = i;
}

// Free the array (must specify the same size)
pfree(nums, 10);
```

### Working with Functions that Return Memory

The pointer-to-pointer interface makes it easy to allocate memory in functions:

```c
void get_data(int** result) {
    // Allocate through a pointer to pointer
    ppalloc(result, 5);
  
    // Initialize the allocated memory
    for (int i = 0; i < 5; i++) {
        (*result)[i] = i * 10;
    }
}

int main() {
    int* data = NULL;
    get_data(&data);
  
    // Use the data
    for (int i = 0; i < 5; i++) {
        printf("%d ", data[i]);  // Outputs: 0 10 20 30 40
    }
  
    // Free the memory
    pfree(data, 5);
  
    return 0;
}
```

### Resizing Arrays

```c
int* data = NULL;
ppalloc(&data, 5);

// Initialize data
for (int i = 0; i < 5; i++) {
    data[i] = i;
}

// Resize the array to 10 elements
pprealloc(&data, 5, 10);

// The first 5 elements are preserved, initialize the rest
for (int i = 5; i < 10; i++) {
    data[i] = i;
}

// Free the resized array
pfree(data, 10);
```

### Custom Allocators

You can define your own allocator implementation before including the header:

```c
// Define your allocator type
typedef struct {
    // Allocator state variables
    void* memory_pool;
    size_t remaining;
} my_allocator_t;

// Create your allocator instance
my_allocator_t my_allocator = {/* initialize */};

// Implement the required allocation functions
void* my_alloc(my_allocator_t* self, size_t alignment, size_t size) {
    // Your allocation logic
}

void* my_realloc(my_allocator_t* self, void* ptr, size_t alignment, 
                size_t old_size, size_t new_size) {
    // Your reallocation logic
}

void my_free(my_allocator_t* self, void const* ptr, size_t alignment, size_t size) {
    // Your deallocation logic
}

// Define the allocator macros before including tgalloc.h
#define TGA (my_allocator_t*)&my_allocator
#define TGA_ALLOC_A_S my_alloc
#define TGA_REALLOC_A_S my_realloc
#define TGA_FREE_A_S my_free

// Now include the header
#include "tgalloc.h"
```

## Allocation Function Interface

When implementing a custom allocator, your functions must adhere to these interfaces:

- **Allocation**: `void* function(allocator_t* self, size_t alignment, size_t size)`

  - `self`: Pointer to your allocator instance
  - `alignment`: Required alignment for the memory (in bytes)
  - `size`: Size to allocate (in bytes)
  - Returns: Pointer to allocated memory or NULL on failure
- **Reallocation**: `void* function(allocator_t* self, void* ptr, size_t alignment, size_t old_size, size_t new_size)`

  - `self`: Pointer to your allocator instance
  - `ptr`: Pointer to memory to reallocate
  - `alignment`: Required alignment for the memory (in bytes)
  - `old_size`: Current size of the memory block (in bytes)
  - `new_size`: Requested new size (in bytes)
  - Returns: Pointer to reallocated memory or NULL on failure
- **Deallocation**: `void function(allocator_t* self, void const* ptr, size_t alignment, size_t size)`

  - `self`: Pointer to your allocator instance
  - `ptr`: Pointer to memory to free
  - `alignment`: Alignment of the memory (in bytes)
  - `size`: Size of the memory block (in bytes)

### Switching Allocators Within a File

One of the great features of tgalloc is the ability to seamlessly switch between different allocators within the same C file using helper header files:

```c
#include "tgalloc.h"

int main() {
    // Using the default allocator (malloc/free based)
    int* data1 = NULL;
    ppalloc(&data1, 10);
  
    // Fill data1 with values
    for (int i = 0; i < 10; i++) {
        data1[i] = i;
    }
  
    // Switch to a custom allocator
    #include "tgalloc_custom.h"
  
    // Now allocations will use the custom allocator
    float* data2 = NULL;
    ppalloc(&data2, 20);
  
    // Fill data2 with values
    for (int i = 0; i < 20; i++) {
        data2[i] = i * 1.5f;
    }
  
    // Free memory with the custom allocator
    pfree(data2, 20);
  
    // Switch back to the default allocator
    #include "tgalloc_default.h"
  
    // This will use the default allocator again
    pfree(data1, 10);
  
    return 0;
}
```

To implement this approach, create helper header files for each allocator:

```c
/* tgalloc_default.h */
#ifndef TGA_DEFAULT_H
#define TGA_DEFAULT_H

#include "tgalloc.h"

#endif // TGA_DEFAULT_H

#undef TGA
#undef TGA_ALLOC_A_S
#undef TGA_REALLOC_A_S
#undef TGA_FREE_A_S

#define TGA (dflt_allo_t*)NULL
#define TGA_ALLOC_A_S _tgalloc_dflt_alloc_aligned_sized
#define TGA_REALLOC_A_S _tgalloc_dflt_realloc_aligned_sized
#define TGA_FREE_A_S _tgalloc_dflt_free_aligned_sized
```

```c
/* tgalloc_custom.h */
#ifndef TGA_CUSTOM_H
#define TGA_CUSTOM_H

#include "tgalloc.h"

// Define your custom allocator
typedef struct {
    // Custom allocator state
    void* memory_pool;
} custom_allocator_t;

// Custom allocator global instance
extern custom_allocator_t custom_allocator;

// Function declarations
void* custom_alloc(custom_allocator_t* self, size_t alignment, size_t size);
void* custom_realloc(custom_allocator_t* self, void* ptr, size_t alignment, 
                   size_t old_size, size_t new_size);
void custom_free(custom_allocator_t* self, void const* ptr, size_t alignment, size_t size);

#endif // TGA_CUSTOM_H

#undef TGA
#undef TGA_ALLOC_A_S
#undef TGA_REALLOC_A_S
#undef TGA_FREE_A_S

#define TGA (custom_allocator_t*)&custom_allocator
#define TGA_ALLOC_A_S custom_alloc
#define TGA_REALLOC_A_S custom_realloc
#define TGA_FREE_A_S custom_free
```

This approach provides a clean way to switch between allocators at any point in your code. Just remember to always free memory using the same allocator that was used to allocate it.

## Compatibility

tgalloc supports both C99 and later standards with progressive enhancements:

- **C99/C11 Compatibility**: The core functionality works with C99 compilers and above
- **C23 Enhancements**: When compiled with C23 support (detected via `__STDC_VERSION__`), tgalloc automatically uses more readable syntax with features like `__VA_OPT__`

The library detects the C standard at compile time and selects the appropriate implementation, ensuring both backward compatibility and taking advantage of newer language features when available.

If alignment utilities like `alignof` is not detected by the codes, the default alignment of a data type is assumed to be either the size of it or `sizeof(void *)`, whichever is smaller.

## License

See `LICENSE` file.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
