#ifndef CONTEXT_H 
#define CONTEXT_H

#include <stdint.h> 

typedef struct context_t {
    uint32_t edi;
    uint32_t esi;
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    uint32_t eax;
    uint32_t ebp;
    uint32_t eip;
} __attribute__((packed)) context_t;

typedef struct interrupt_frame_t {
    uint32_t ds;
    //pusha/popa
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp; //ignored
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    //interrupt values
    uint32_t interrupt_number;
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    //only valid when a privilege change occurs
    uint32_t user_esp;
    uint32_t ss;
} __attribute__((packed)) interrupt_frame_t;

#endif //CONTEXT_H

