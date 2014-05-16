#include "string.h"

int strcmp(const char* s1, const char* s2) {
    while(*s1 && *s1 == *s2)
        s1++, s2++;

    return *s1 - *s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while(*s1 && *s1 == *s2 && n--)
        s1++, s2++;

    return *s1 - *s2;
}

char* strcpy(char* dst, const char* src) {
    char* d = dst;

    while((*dst++ = *src++) != 0)
        ;

    return d;
}

char* strncpy(char* dst, const char* src, size_t n) {
    char* d = dst;

    while((*dst++ = *src++) != 0 && n--)
        ;

    return d;
}

size_t strlen(const char* str) {
    size_t len = 0;

    while(*str++)
        len++;
    
    return len;
}

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
