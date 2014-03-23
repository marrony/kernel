#include <stdint.h>
#include <string.h>
#include "irq.h"
#include "heap.h"
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
#define ADDRESS(x)   ((x) & 0xfffff000)

#define IS_PROTECTION(x)  ((x) & (1 << 0))
#define IS_NONPRESENT(x)  (!IS_PROTECTION(x))
#define IS_WRITE(x)       ((x) & (1 << 1))
#define IS_USER(x)        ((x) & (1 << 2))
#define IS_RESERVED(x)    ((x) & (1 << 3))
#define IS_INSTRUCTION(x) ((x) & (1 << 4))

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

#define BLOCK_SIZE       4096

void mm_alloc_page(uint32_t* page_table_entry) {
    if(ADDRESS(*page_table_entry) != 0)
        return;

    uint32_t page = (uint32_t)kamalloc(BLOCK_SIZE, 4096);
    *page_table_entry |= PRESENT | READ | USER | page;
}

void mm_free_page(uint32_t* page_table_entry) {
    uint32_t page = ADDRESS(*page_table_entry);
    kfree((void*)page);
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
        page_table_t* page_table = (page_table_t*)kamalloc(sizeof(page_table_t), 4096);

        memset(page_table, 0, sizeof(page_table_t));
        directory->tables[table_index] = PRESENT | READ | USER | (uint32_t)page_table; 

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

    if(IS_PROTECTION(regs->error_code))
        kprintf("The fault was caused by a page-level protection violation\n");
    else
        kprintf("The fault was caused by a non-present page\n");

    if(IS_WRITE(regs->error_code))
        kprintf("The access causing the fault was a write\n");
    else
        kprintf("The access causing the fault was a read\n");

    if(IS_USER(regs->error_code))
        kprintf("A user-mode access caused the fault\n");
    else
        kprintf("A kernel-mode access caused the fault\n");

    if(IS_RESERVED(regs->error_code))
        kprintf("The fault was caused by a reserved bit set to 1 in some paging-structure entry\n");

    if(IS_INSTRUCTION(regs->error_code))
        kprintf("The fault was caused by an instruction fetch\n");

    while(1)
        hlt();
}

void init_paging(uint32_t max_memory) {
    init_kernel_heap(max_memory);

    kernel_directory = (page_directory_t*)kamalloc(sizeof(page_directory_t), 4096);
    memset(kernel_directory, 0, sizeof(page_directory_t));

    for(uint32_t ptr = 0; ptr < max_memory; ptr += BLOCK_SIZE) {
        mm_remap(kernel_directory, ptr, ptr);
    }

    register_interrupt_handler(14, &page_fault_handler);
    mm_switch_page_directory(kernel_directory);
}

