#include <stdint.h>
#include <string.h>

#include "paging.h"
#include "interrupt.h"
#include "heap.h"
#include "task.h"
#include "asm.h"
#include "global.h"
#include "kernel.h"

#define PAGE_SIZE    4096

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

struct pde_t* kernel_directory;
struct pde_t* current_directory;

void alloc_page(uint32_t* page_table_entry) {
    if(ADDRESS(*page_table_entry) != 0)
        return;

    uint32_t page = (uint32_t)kamalloc(PAGE_SIZE, 4096);
    *page_table_entry |= PRESENT | READ | USER | page;
}

void free_page(uint32_t* page_table_entry) {
    uint32_t page = ADDRESS(*page_table_entry);
    kfree((void*)page);
    *page_table_entry = 0;
}

uint32_t* get_page_entry(struct pde_t* directory, uint32_t address, int create_page) {
    uint32_t table_index = address >> 22;
    uint32_t page_index = (address >> 12) & 0x3ff;

    if(ADDRESS(directory->tables[table_index]) != 0) {
        struct pte_t* page_table = (struct pte_t*)ADDRESS(directory->tables[table_index]);

        return &page_table->pages[page_index];
    }

    if(create_page) {
        struct pte_t* page_table = (struct pte_t*)kamalloc(sizeof(struct pte_t), 4096);

        memset(page_table, 0, sizeof(struct pte_t));
        directory->tables[table_index] = PRESENT | READ | USER | (uint32_t)page_table; 

        return &page_table->pages[page_index];
    }

    return 0;
}

int remap(struct pde_t* directory, uint32_t physical, uint32_t virtual) {
    uint32_t* page_entry = get_page_entry(directory, virtual, 1);

    if(page_entry) {
        *page_entry |= PRESENT | USER | READ | physical;
        return 0;
    }

    return 1;
}

static void enable_paging() {
    uint32_t cr0;
    __asm__ __volatile__ ("movl %%cr0, %0" : "=r"(cr0));
    __asm__ __volatile__ ("movl %0, %%cr0" : : "r"(cr0 | 0x80000000));
}

static void disable_paging() {
    uint32_t cr0;
    __asm__ __volatile__ ("movl %%cr0, %0" : "=r"(cr0));
    __asm__ __volatile__ ("movl %0, %%cr0" : : "r"(cr0 & 0x7fffffff));
}

void switch_page_directory(struct pde_t* directory) {
    __asm__ __volatile__ ("movl %0, %%cr3" : : "r"(directory));
    current_directory = directory;

    enable_paging();
}

struct pte_t* clone_page_table(const struct pte_t* page) {
    struct pte_t* new_page = (struct pte_t*)kamalloc(sizeof(struct pte_t), 4096);
    memset(new_page, 0, sizeof(struct pte_t)); 

    for(int i = 0; i < 1024; i++) {
        if(!ADDRESS(page->pages[i]))
            continue;

        alloc_page(&new_page->pages[i]);
        new_page->pages[i] |= (page->pages[i] & 0xfff);

        cli();
        disable_paging();
        memcpy((void*)ADDRESS(new_page->pages[i]), (void*)ADDRESS(page->pages[i]), PAGE_SIZE);
        enable_paging();
        sti();
    }

    return new_page;
}

struct pde_t* clone_page_directory(const struct pde_t* directory) {
    struct pde_t* new_directory = kamalloc(sizeof(struct pde_t), 4096);
    memset(new_directory, 0, sizeof(struct pde_t));

    for(int i = 0; i < 1024; i++) {
        if(!directory->tables[i])
            continue;

        if(ADDRESS(kernel_directory->tables[i]) == ADDRESS(directory->tables[i])) {
            new_directory->tables[i] = directory->tables[i];
        } else {
            struct pte_t* page_table = clone_page_table((struct pte_t*)ADDRESS(directory->tables[i]));
            new_directory->tables[i] = PRESENT | USER | READ | (uint32_t)page_table; 
        }
    }

    return new_directory;
}

uint32_t get_mapping(struct pde_t* directory, uint32_t address) {
    uint32_t table_index = (address >> 22) & 0x3ff;
    uint32_t page_index = (address >> 12) & 0x3ff;
    uint32_t frame_index = address & 0xfff;

    uint32_t table_entry = directory->tables[table_index];
    if(ADDRESS(table_entry) == 0)
        return ~0;

    struct pte_t* page_table = (struct pte_t*)ADDRESS(table_entry);

    uint32_t page_entry = page_table->pages[page_index];
    if(ADDRESS(page_entry) == 0)
        return ~0;

    return ADDRESS(page_entry) | frame_index;
}

void page_fault_handler() {
    uint32_t address;

    __asm__ __volatile__ ("movl %%cr2, %0" : "=r"(address));

    kprintf("A page fault was caught at address 0x%x\n", address);

    struct trap_t* frame = current_task->trap;

    if(IS_PROTECTION(frame->error_code))
        kprintf("The fault was caused by a page-level protection violation\n");
    else
        kprintf("The fault was caused by a non-present page\n");

    if(IS_WRITE(frame->error_code))
        kprintf("The access causing the fault was a write\n");
    else
        kprintf("The access causing the fault was a read\n");

    if(IS_USER(frame->error_code))
        kprintf("A user-mode access caused the fault\n");
    else
        kprintf("A kernel-mode access caused the fault\n");

    if(IS_RESERVED(frame->error_code))
        kprintf("The fault was caused by a reserved bit set to 1 in some paging-structure entry\n");

    if(IS_INSTRUCTION(frame->error_code))
        kprintf("The fault was caused by an instruction fetch\n");

    while(1)
        hlt();
}

void init_paging(uint32_t max_memory) {
    init_kernel_heap(max_memory);

    kernel_directory = (struct pde_t*)kamalloc(sizeof(struct pde_t), 4096);
    memset(kernel_directory, 0, sizeof(struct pde_t));

    for(uint32_t ptr = 0; ptr < max_memory; ptr += PAGE_SIZE) {
        remap(kernel_directory, ptr, ptr);
    }

    register_interrupt_handler(14, &page_fault_handler);
    switch_page_directory(kernel_directory);
}

