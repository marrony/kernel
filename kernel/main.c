#include "inttypes.h"
#include "stdarg.h"
#include "io.h"

int16_t* video_memory = (int16_t*)0xb8000;
int16_t video_x_position = 0;
int16_t video_y_position = 0;
int16_t video_attribute = 0x0f00;
int16_t video_width = 80;
int16_t video_height = 25;
int8_t video_tab_size = 8;

#define LINEAR(x, y) ((y)*video_width + (x))

void gotoxy() {
    int16_t location = LINEAR(video_x_position, video_y_position);
    outb(0x3d4, 14);
    outb(0x3d5, location >> 8);
    outb(0x3d4, 15);
    outb(0x3d5, location & 0xff);
}

void putch(char ch) {
    if(ch == 0x08 && video_x_position > 0) {
        video_x_position--;
    } else if(ch == 0x09) {
        video_x_position = (video_x_position + video_tab_size) & ~(video_tab_size - 1);
    } else if(ch == '\r') {
        video_x_position = 0;
    } else if(ch == '\n') {
        video_x_position = 0;
        video_y_position++;
    } else {
        int16_t linear = LINEAR(video_x_position, video_y_position);
        video_memory[linear] = video_attribute | ch;
        video_x_position++;
    }

    if(video_x_position >= video_width) {
        video_x_position = 0;
        video_y_position++;
    }

    if(video_y_position >= video_height) {
        int16_t empty = video_attribute | ' ';

        for(int line = 0; line < video_height-1; line++) {
            int16_t* line0 = video_memory + LINEAR(0, line+0); 
            int16_t* line1 = video_memory + LINEAR(0, line+1);

            for(int x = 0; x < video_width; x++)
                line0[x] = line1[x];
        }

        int16_t* last_line = video_memory + LINEAR(0, video_height-1);
        for(int x = 0; x < video_width; x++)
            last_line[x] = empty;

        video_y_position--;
    }

    gotoxy();
}

void puts(const char* str) {
    while(*str) {
        putch(*str++);
    }
}

int vsnprintf(char* buffer, size_t size, const char* fmt, va_list va) {
    //TODO
    return 0;
}

int printf(const char* fmt, ...) {
    char buffer[1024 + 1];
    va_list va;
    int ret;

    va_start(va, fmt);
    ret = vsnprintf(buffer, sizeof(buffer) - 1, fmt, va);
    va_end(va);

    puts(buffer);
    return ret;
}

int main() {
    puts("THIS IS MY AWESOME KERNEL\n");
    puts("AUTHOR: MARRONY N. NERIS\n");
    puts("VERSION: 1.0");
    return 0;
}
