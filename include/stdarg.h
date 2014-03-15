#ifndef STDARG_H
#define STDARG_H

#include <stdint.h> 

typedef void* va_list;

#define va_start(list, param) \
    (list = (va_list) ((int32_t*)&param + 1))

#define va_arg(list, type)   \
     (*(type*) ((int32_t*)(list = (int32_t*)list + 1) - 1))

#define va_end(list) \
    (list = 0)

#endif //STDARG_H
