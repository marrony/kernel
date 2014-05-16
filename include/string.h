#ifndef STRING_H
#define STRING_H

#include <stdint.h> 

size_t strlen(const char* str);

int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, size_t n);

void* memset(void* ptr, int byte, size_t size);

void* memcpy(void* dst, const void* src, size_t size);

#endif //STRING_H
