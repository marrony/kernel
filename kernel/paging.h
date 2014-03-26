#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

typedef struct page_table_t {
    uint32_t pages[1024];
} page_table_t;

typedef struct page_directory_t {
    uint32_t tables[1024];
} page_directory_t;

#define PAGE_SIZE 4096

extern page_directory_t* kernel_directory;
extern page_directory_t* current_directory;

void mm_alloc_page(uint32_t* page_table_entry);
void mm_free_page(uint32_t* page_table_entry);

uint32_t* mm_get_page_entry(page_directory_t* directory, uint32_t address, int create_page);
int mm_remap(page_directory_t* directory, uint32_t physical, uint32_t virtual);

void mm_switch_page_directory(page_directory_t* directory);
page_table_t* mm_clone_page_table(const page_table_t* page);
page_directory_t* mm_clone_page_directory(const page_directory_t* directory);

uint32_t mm_get_mapping(page_directory_t* directory, uint32_t address);

void init_paging(uint32_t max_memory);

#endif //PAGING_H

