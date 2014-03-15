#include "string.h"

void* memset(void* ptr, int byte, size_t size) {
    char* p = (char*)ptr;
    for(size_t i = 0; i < size; i++)
        *p++ = byte;
    return ptr;
}
