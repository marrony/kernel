#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

struct pte_t {
    uint32_t pages[1024];
};

struct pde_t {
    uint32_t tables[1024];
};

#define PAGE_SIZE 4096

void mm_alloc_page(uint32_t* page_table_entry);
void mm_free_page(uint32_t* page_table_entry);

uint32_t* mm_get_page_entry(struct pde_t* directory, uint32_t address, int create_page);
int mm_remap(struct pde_t* directory, uint32_t physical, uint32_t virtual);

void mm_switch_page_directory(struct pde_t* directory);
struct pte_t* mm_clone_page_table(const struct pte_t* page);
struct pde_t* mm_clone_page_directory(const struct pde_t* directory);

uint32_t mm_get_mapping(struct pde_t* directory, uint32_t address);

void init_paging(uint32_t max_memory);

extern struct pde_t* current_directory;

#endif //PAGING_H

