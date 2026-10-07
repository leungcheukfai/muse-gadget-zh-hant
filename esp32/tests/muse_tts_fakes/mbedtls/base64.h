#pragma once

#include <stddef.h>

int mbedtls_base64_decode(unsigned char *dst, size_t dst_len, size_t *olen,
                          const unsigned char *src, size_t src_len);
