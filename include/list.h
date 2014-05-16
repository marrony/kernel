#ifndef LIST_H
#define LIST_H

struct list_t {
    struct list_t* next;
    struct list_t* prev;
};

static inline void list_init(struct list_t* head) {
    head->prev = head;
    head->next = head;
}

static inline void _list_add(struct list_t* new, struct list_t* prev, struct list_t* next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void list_add(struct list_t* head, struct list_t* new) {
    _list_add(new, head, head->next);
}

static inline void list_append(struct list_t* head, struct list_t* new) {
    _list_add(new, head->prev, head);
}

static inline void _list_remove(struct list_t* prev, struct list_t* next) {
    next->prev = prev;
    prev->next = next;
}

static inline void list_remove(struct list_t* head) {
    _list_remove(head->prev, head->next);
}

#endif //LIST_H

