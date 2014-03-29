#include "timer.h"
#include "asm.h"

#define PIT_COUNTER_DIVISOR 0x40
#define PIT_REFRESH_COUNTER 0x41
#define PIT_SPEAKER         0x42
#define PIT_MODE            0x43
#define PIT_BINARY          (0 << 0)
#define PIT_BCD             (1 << 0)
#define PIT_ONE_SHOT        (1 << 1)
#define PIT_RATE_GENERATOR  (2 << 1)
#define PIT_SQUARE_WAVE     (3 << 1)
#define PIT_SOFT_STROBE     (4 << 1)
#define PIT_HARD_STROBE     (5 << 1)
#define PIT_FIRST_BYTE      (1 << 4)
#define PIT_SECOND_BYTE     (2 << 4)
#define PIT_TWO_BYTE        (PIT_FIRST_BYTE | PIT_SECOND_BYTE)
#define PIT_COUNTER0        (0 << 6)
#define PIT_COUNTER1        (1 << 6)
#define PIT_COUNTER2        (2 << 6)

// initialize Programmable Interval Timer (i8254)
void init_timer(uint32_t frequency) {
    uint32_t divisor = 1193182 / frequency;

    outb(PIT_MODE, PIT_TWO_BYTE | PIT_SQUARE_WAVE | PIT_BINARY);
    outb(PIT_COUNTER_DIVISOR, divisor & 0xff);
    outb(PIT_COUNTER_DIVISOR, (divisor >> 8) & 0xff);
}

