#include <stdint.h>
#include <string.h>
#include "heap.h"
#include "kprintf.h"

struct header_t {
    uint32_t magic;
    uint8_t is_hole;
    size_t size;
    struct header_t* next;
    struct header_t* previous;
};

struct footer_t {
    uint32_t magic;
    struct header_t* header;
};

static struct header_t* remove_list(struct header_t* head, struct header_t* node) {
    if(node == head) {
        head = head->next;
        if(head) head->previous = 0;
        return head;
    }

    struct header_t* next = node->next;
    struct header_t* previous = node->previous;

    if(previous) previous->next = next;
    if(next) next->previous = previous;

    return head;
}

static struct header_t* insert_list(struct header_t* list, struct header_t* node) {
    node->next = 0;
    node->previous = 0;

    if(!list)
        return node;
    
    struct header_t* actual = list;
    struct header_t* previous = 0;

    while(actual) {
        if(actual->size >= node->size)
            break;

        previous = actual;
        actual = actual->next;
    }

    if(list == actual) {
        list->previous = node;
        node->next = list;
        return node;
    }

    previous->next = node;
    node->previous = previous;

    if(actual) {
        node->next = actual;
        actual->previous = node;
    }

    return list;
}

static struct header_t* heap_find_smallest(struct heap_t* heap, size_t size, size_t align) {
    struct header_t* actual = heap->free_list;

    while(actual != 0) {
        uint32_t offset = 0;

        if(align > 0) {
            uint32_t address = (uint32_t)actual + sizeof(struct header_t);
            uint32_t aligned_address = (address + align - 1) & ~(align - 1);
            offset = aligned_address - address;
        }

        if(size+offset <= actual->size)
            return actual;

        actual = actual->next;
    }


    return 0;
}

static struct header_t* create_header(uint32_t address, size_t size) {
    struct header_t* header = (struct header_t*)address;
    struct footer_t* footer = (struct footer_t*)(address + size - sizeof(struct footer_t));

    header->size = size;
    header->magic = 0xdeadbeef;
    header->is_hole = 1;
    header->next = 0;
    header->previous = 0;

    footer->magic = 0xdeadbeef;
    footer->header = header;

    return header;
}

void* heap_alloc(struct heap_t* heap, size_t size, size_t align) {
    size_t padding_size = sizeof(struct header_t) + sizeof(struct footer_t);
    size_t required_size = size + padding_size; 

    struct header_t* smallest = heap_find_smallest(heap, required_size, align);
    uint32_t address = (uint32_t)smallest;

    if(smallest == 0)
        return 0;

    heap->free_list = remove_list(heap->free_list, smallest);

    if(smallest->size - required_size < padding_size) {
        size += smallest->size - required_size;
        required_size = smallest->size;
    }

    uint32_t block_size = smallest->size;

    if(align > 0) {
        uint32_t old_address = address + sizeof(struct header_t);
        uint32_t aligned_address = (old_address + align - 1) & ~(align - 1);

        if(old_address != aligned_address) {
            uint32_t left_size = aligned_address - old_address;

            struct header_t* left_header = create_header(address, left_size);

            heap->free_list = insert_list(heap->free_list, left_header);

            address = aligned_address - sizeof(struct header_t);
            block_size -= left_size;
        }
    }

    struct header_t* new_header = create_header(address, required_size);
    new_header->is_hole = 0;

    if(block_size - required_size > 0) {
        uint32_t right_size = block_size - required_size;

        struct header_t* right_header = create_header(address + required_size, right_size);

        heap->free_list = insert_list(heap->free_list, right_header);
    }

    return (void*)(address + sizeof(struct header_t));
}

void heap_free(struct heap_t* heap, void* ptr) {
    if(!ptr) return;

    struct header_t* header = (struct header_t*)((uint32_t)ptr - sizeof(struct header_t));
    struct footer_t* footer = (struct footer_t*)((uint32_t)header + header->size - sizeof(struct footer_t));

    struct footer_t* left_footer = (struct footer_t*)((uint32_t)header - sizeof(struct footer_t));
    struct header_t* left_header = left_footer->header;
    int merge_left = (left_header->magic == 0xdeadbeef) && (left_header->is_hole == 1);

    struct header_t* right_header = (struct header_t*)((uint32_t)header + header->size);
    struct footer_t* right_footer = (struct footer_t*)((uint32_t)right_header + right_header->size - sizeof(struct footer_t));
    int merge_right = (right_header->magic == 0xdeadbeef) && (right_header->is_hole == 1);

    header->is_hole = 1;

    if(merge_left && merge_right) {
        heap->free_list = remove_list(heap->free_list, left_header);
        heap->free_list = remove_list(heap->free_list, right_header);

        left_header->size += header->size + right_header->size;
        right_footer->header = left_header;

        heap->free_list = insert_list(heap->free_list, left_header);
    } else if(merge_left) {
        heap->free_list = remove_list(heap->free_list, left_header);

        left_header->size += header->size;
        footer->header = left_header;

        heap->free_list = insert_list(heap->free_list, left_header);
    } else if(merge_right) {
        heap->free_list = remove_list(heap->free_list, right_header);

        header->size += right_header->size;
        right_footer->header = header;

        heap->free_list = insert_list(heap->free_list, header);
    } else {
        heap->free_list = insert_list(heap->free_list, header);
    }
}

static struct heap_t* kernel_heap = 0;

void print_free() {
    struct header_t* h = kernel_heap->free_list;

    while(h) {
        kprintf("%x %x => is_hole: %d, size: %x, prev: %x, next: %x\n",
             h, (uint32_t)h + h->size - sizeof(struct footer_t), h->is_hole, h->size,
             h->previous, h->next);
        h = h->next;
    }
}

void* kamalloc(size_t size, size_t align) {
    return heap_alloc(kernel_heap, size, align);
}

void* kmalloc(size_t size) {
    return kamalloc(size, 0);
}

void kfree(void* ptr) {
    heap_free(kernel_heap, ptr);
}

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

void init_kernel_heap(uint32_t max_memory) {
    kernel_heap = (struct heap_t*)current_memory;
    current_memory += sizeof(struct heap_t);
    current_memory += sizeof(struct footer_t);

    struct header_t* header = create_header(current_memory, max_memory - current_memory);
    kernel_heap->free_list = insert_list(0, header);
}

