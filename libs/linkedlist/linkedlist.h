#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>

struct node {
    void *data;
    struct node *next;
    struct node *prev;
};

struct linkedlist {
    struct node *start;
    struct node *end;
    size_t len;
    size_t elemsize;
};

struct linkedlist *linkedlist_init(size_t elemsize);

void linkedlist_insert_start(struct linkedlist *list, const void *data);
void linkedlist_insert_end(struct linkedlist *list, const void *data);
void linkedlist_insert_after(struct linkedlist *list, struct node *base,
                             const void *data);
void linkedlist_insert_before(struct linkedlist *list, struct node *base,
                              const void *data);

void linkedlist_bypass(struct linkedlist *list, struct node *del);
void linkedlist_free_node(struct linkedlist *list, struct node *n);
void linkedlist_delete(struct linkedlist *list, struct node *del);

void linkedlist_free(struct linkedlist *list);

#endif
