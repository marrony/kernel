#ifndef HEAP_H
#define HEAP_H

struct heap_t;

void* heap_alloc(struct heap_t* heap, size_t size, size_t align);
void heap_free(struct heap_t* heap, void* ptr);

struct heap_t* create_heap();
void destroy_heap(struct heap_t* heap);

void* kamalloc(size_t size, size_t align);
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif //HEAP_H
