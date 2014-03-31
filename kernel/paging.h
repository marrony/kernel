#ifndef PAGING_H
#define PAGING_H

struct pte_t {
    uint32_t pages[1024];
};

struct pde_t {
    uint32_t tables[1024];
};

void switch_page_directory(struct pde_t* directory);
struct pde_t* clone_page_directory(const struct pde_t* directory);
uint32_t get_mapping(struct pde_t* directory, uint32_t address);

#endif //PAGING_H

