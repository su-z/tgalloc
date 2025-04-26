#ifndef TGA_DEFAULT_H
#define TGA_DEFAULT_H

#include "tgalloc.h"

#endif // TGA_DEFAULT_H

#undef ALLO
#undef TGA_ALLOC_A_S
#undef TGA_REALLOC_A_S
#undef TGA_FREE_A_S

#define ALLO (dflt_allo_t*)NULL
#define TGA_ALLOC_A_S _tgalloc_dflt_alloc_aligned_sized
#define TGA_REALLOC_A_S _tgalloc_dflt_realloc_aligned_sized
#define TGA_FREE_A_S _tgalloc_dflt_free_aligned_sized
