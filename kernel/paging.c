#include <stdint.h>
#include <string.h>

#include "irq.h"
#include "asm.h"

#define PRESENT      (1 << 0)
#define READ         (1 << 1)
#define USER         (1 << 2)
#define PWT          (1 << 3)
#define PCD          (1 << 4)
#define ACCESSED     (1 << 5)
#define DIRTY        (1 << 6)
#define PAT          (1 << 7)
#define GLOBAL       (1 << 8)
#define AVAILABLE(x) ((x) << 9)
#define FRAME(x)     ((x) << 12)
#define BLOCK(x)     ((x) >> 12)
#define ADDRESS(x)   ((x) & 0xfffff000)

/*typedef struct {
    uint32_t present  : 1;
    uint32_t rw       : 1;
    uint32_t user     : 1;
    uint32_t pwt      : 1;
    uint32_t pcd      : 1;
    uint32_t accessed : 1;
    uint32_t dirty    : 1;
    uint32_t pat      : 1;
    uint32_t global   : 1;
    uint32_t ignored  : 3;
    uint32_t block    : 20;
} page_table_entry_t, page_directory_entry_t;*/

typedef struct page_table_t {
    uint32_t pages[1024];
} page_table_t;

typedef struct page_directory_t {
    uint32_t tables[1024];
} page_directory_t;

extern char __kernel_start;
extern char __kernel_end;
extern char __text_start;
extern char __text_end;
extern char __data_start;
extern char __data_end;
extern char __bss_start;
extern char __bss_end;
extern char __stack;

static uint32_t current_memory = (uint32_t) &__kernel_end;

void* kamalloc(size_t size, size_t align) {
    uint32_t ptr = current_memory;
    if(align != 0)
        ptr = (ptr + align - 1) & ~(align - 1);
    current_memory = ptr + size;
    return (void*)ptr;
}

void* kmalloc(size_t size) {
    return kamalloc(size, 0);
}

#define BLOCK_SIZE       4096
#define BLOCK_INDEX(x)   ((x) / 32)
#define BLOCK_OFFSET(x)  ((x) % 32)

uint32_t mm_max_blocks;
uint32_t* mm_block_map; 

void mm_block_set(uint32_t block) {
    mm_block_map[BLOCK_INDEX(block)] |= (1 << BLOCK_OFFSET(block));
}

void mm_block_unset(uint32_t block) {
    mm_block_map[BLOCK_INDEX(block)] &= ~(1 << BLOCK_OFFSET(block));
}

int mm_block_isset(uint32_t block) {
    return mm_block_map[BLOCK_INDEX(block)] & (1 << BLOCK_OFFSET(block));
}

uint32_t mm_get_first_free() {
    for(uint32_t i = 0; i < BLOCK_INDEX(mm_max_blocks); i++) {
        uint32_t bits = mm_block_map[i];

        if(bits == 0xffffffff)
            continue;

        for(int j = 0; j < 32; j++) {
            int bit = 1 << j;

            if(!(bits & bit))
                return i*32 + j;
        }
    }

    return 0xffffffff;
}

void mm_mark_blocks(uint32_t ptr, size_t nblocks) {
    ptr /= BLOCK_SIZE;

    for(size_t i = 0; i < nblocks; i++)
        mm_block_set(ptr++);
}

void mm_mark_used_blocks(uint32_t start, uint32_t end) {
    for(uint32_t i = start; i < end; i += BLOCK_SIZE)
        mm_block_set(i / BLOCK_SIZE);
}

uint32_t mm_get_block() {
    uint32_t free = mm_get_first_free();

    if(free == 0xffffffff)
        return 0;

    mm_block_set(free);

    return free;
}

void mm_alloc_page(uint32_t* page_table_entry) {
    if(ADDRESS(*page_table_entry) != 0)
        return;

    uint32_t block = mm_get_block();
    *page_table_entry |= PRESENT | READ | USER | FRAME(block);
}

void mm_free_page(uint32_t* page_table_entry) {
    mm_block_unset(BLOCK(*page_table_entry));
    *page_table_entry = 0;
}

uint32_t* mm_get_page_entry(page_directory_t* directory, uint32_t address, int create_page) {
    uint32_t table_index = address >> 22;
    uint32_t page_index = (address >> 12) & 0x3ff;

    if(ADDRESS(directory->tables[table_index]) != 0) {
        page_table_t* page_table = (page_table_t*)ADDRESS(directory->tables[table_index]);
        return &page_table->pages[page_index];
    }

    if(create_page) {
        uint32_t page_block = mm_get_block();
        page_table_t* page_table = (page_table_t*)FRAME(page_block);

        memset(page_table, 0, sizeof(page_table_t));
        directory->tables[table_index] = PRESENT | READ | USER | FRAME(page_block);

        return &page_table->pages[page_index];
    }

    return 0;
}

int mm_remap(page_directory_t* directory, uint32_t physical, uint32_t virtual) {
    uint32_t* page_entry = mm_get_page_entry(directory, virtual, 1);

    if(page_entry) {
        *page_entry |= PRESENT | USER | READ | physical;
        return 0;
    }

    return 1;
}

void mm_switch_page_directory(page_directory_t* directory) {
    __asm__ __volatile__ ("movl %0, %%cr3" : : "r"(directory));

    uint32_t cr0;
    __asm__ __volatile__ ("movl %%cr0, %0" : "=r"(cr0));
    __asm__ __volatile__ ("movl %0, %%cr0" : : "r"(cr0 | 0x80000000));
}

uint32_t mm_get_mapping(page_directory_t* directory, uint32_t address) {
    uint32_t table_index = (address >> 22) & 0x3ff;
    uint32_t page_index = (address >> 12) & 0x3ff;
    uint32_t frame_index = address & 0xfff;

    uint32_t table_entry = directory->tables[table_index];
    if(ADDRESS(table_entry) == 0)
        return ~0;

    page_table_t* page_table = (page_table_t*)ADDRESS(table_entry);

    uint32_t page_entry = page_table->pages[page_index];
    if(ADDRESS(page_entry) == 0)
        return ~0;

    return ADDRESS(page_entry) | frame_index;
}

page_directory_t* kernel_directory;

#include "kprintf.h"

void page_fault_handler(const struct registers_t* regs) {
    uint32_t address;

    __asm__ __volatile__ ("movl %%cr2, %0" : "=r"(address));

    kprintf("A page fault was caught at address 0x%x\n", address);

    while(1)
        hlt();
}

void init_paging(uint32_t max_memory) {
    mm_mark_used_blocks(0, current_memory);

    mm_max_blocks = max_memory / BLOCK_SIZE;
    mm_block_map = (uint32_t*)FRAME(mm_get_block());
    kernel_directory = (page_directory_t*)mm_get_block();
    memset(kernel_directory, 0, sizeof(page_directory_t));

    for(uint32_t ptr = 0; ptr < max_memory; ptr += BLOCK_SIZE) {
        mm_remap(kernel_directory, ptr, ptr);
    }

    register_interrupt_handler(14, &page_fault_handler);
    mm_switch_page_directory(kernel_directory);
}

