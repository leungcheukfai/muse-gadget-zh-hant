#pragma once

#include <stdlib.h>

#define MALLOC_CAP_SPIRAM 1
#define MALLOC_CAP_8BIT 2

static inline void *heap_caps_realloc(void *ptr, size_t size, int caps)
{
    (void)caps;
    return realloc(ptr, size);
}

static inline void heap_caps_free(void *ptr)
{
    free(ptr);
}
