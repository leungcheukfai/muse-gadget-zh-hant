#pragma once

#include_next <string.h>

#ifndef __APPLE__
size_t strlcpy(char *dst, const char *src, size_t dst_size);
#endif
