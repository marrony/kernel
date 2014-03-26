#include "string.h"

void* memset(void* ptr, int byte, size_t size) {
    char* p = (char*)ptr;
    for(size_t i = 0; i < size; i++)
        *p++ = byte;
    return ptr;
}

void* memcpy(void* dst, const void* src, size_t size) {
    char* d = (char*)dst;
    const char* s = (const char*)src;
    for(size_t i = 0; i < size; i++)
        *d++ = *s++;
    return dst;
}
