#ifndef ASM_H
#define ASM_H

#define cli() __asm__ __volatile__("cli")
#define sti() __asm__ __volatile__("sti")
#define hlt() __asm__ __volatile__("hlt")

//out %ax, port
//out %ax, %dx

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__ ("outb %1, %0" : : "dN"(port), "a"(value));
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ __volatile__ ("outw %1, %0" : : "dN"(port), "a"(value));
}

static inline void outl(uint16_t port, uint32_t value) {
    __asm__ __volatile__ ("outl %1, %0" : : "dN"(port), "a"(value));
}

//in port, %ax
//in %dx, %ax

static inline uint8_t inb(uint16_t port) {
    uint8_t value = 0;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(value) : "dN"(port));
    return value;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ __volatile__ ("inw %1, %0" : "=a"(value) : "dN"(port));
    return value;
}

static inline uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ __volatile__ ("inl %1, %0" : "=a"(value) : "dN"(port));
    return value;
}

#endif //ASM_H

