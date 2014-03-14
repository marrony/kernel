#include "stdarg.h"

static void itoa(char* buffer, int32_t value) {
    char* ptr = buffer;

    if(value < 0) {
        *ptr++ = '-';
        buffer++;
        value = -value;
    }

    do {
        int remainder = value % 10;
        *ptr++ = remainder + '0';
    } while(value /= 10);

    *ptr = 0;

    char* p1 = buffer;
    char* p2 = ptr - 1;
    while(p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

static void uitoa(char* buffer, uint32_t value) {
    itoa(buffer, value);
}

static void hextoa(char* buffer, uint32_t value) {
    static const char hex[] = "0123456789abcdef";
    const char* x = (const char*)&value;

    for(int i = 3; i >= 0; i--) {
        char ch = x[i];
        *buffer++ = hex[(ch >> 4) & 0xf];
        *buffer++ = hex[ch & 0xf];
    }

    *buffer = 0;
}

int vsnprintf(char* output, size_t size, const char* fmt, va_list va) {
    char buffer[32];
    const char* ptr;
    char ch;

    while((ch = *fmt++) != 0 && size > 0) {
        if(ch != '%') {
            *output++ = ch;
            continue;
        }

        ch = *fmt++;
        switch(ch) {
            case 'd':
                itoa(buffer, va_arg(va, int32_t));
                ptr = buffer;
                break;

            case 'u':
                uitoa(buffer, va_arg(va, uint32_t));
                ptr = buffer;
                break;

            case 'x':
                hextoa(buffer, va_arg(va, uint32_t));
                ptr = buffer;
                break;

            case 's':
                ptr = va_arg(va, const char*);
                if(!ptr) ptr = "(null)";
                break;

            default:
                buffer[0] = 0;
                ptr = buffer;
                break;
        }

        while(*ptr && size > 0) {
            *output++ = *ptr++;
            size--;
        }
    }

    *output = 0;

    return 0;
}

