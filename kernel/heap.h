#ifndef HEAP_H
#define HEAP_H

typedef struct header_t header_t;

typedef struct {
    header_t* free_list;
} heap_t;

void init_kernel_heap(uint32_t max_memory);

void* heap_alloc(heap_t* heap, size_t size, size_t align);
void heap_free(heap_t* heap, void* ptr);

heap_t* create_heap();
void destroy_heap(heap_t* heap);

void* kamalloc(size_t size, size_t align);
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif //HEAP_H
