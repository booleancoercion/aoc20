#include "linkedlist.h"
#include <string.h>

typedef struct linkedlist llist;
typedef struct node node;

llist *linkedlist_init(size_t elemsize) {
    llist *list = malloc(sizeof(llist));

    list->start = NULL;
    list->end = NULL;
    list->len = 0;
    list->elemsize = elemsize;

    return list;
}

static node *newnode(llist *list, const void *data) {
    node *n = malloc(sizeof(node));

    n->data = malloc(list->elemsize);
    memcpy(n->data, data, list->elemsize);

    return n;
}

static void first_node(llist *list, const void *data) {
    node *n = newnode(list, data);

    n->next = NULL;
    n->prev = NULL;
    list->start = n;
    list->end = n;
    list->len = 1;
}

void linkedlist_insert_start(llist *list, const void *data) {
    if(list->len == 0) {
        first_node(list, data);
    } else {
        linkedlist_insert_before(list, list->start, data);
    }
}

void linkedlist_insert_end(llist *list, const void *data) {
    if(list->len == 0) {
        first_node(list, data);
    } else {
        linkedlist_insert_after(list, list->end, data);
    }
}

void linkedlist_insert_after(llist *list, node *base, const void *data) {
    node *n = newnode(list, data);

    n->next = base->next;
    n->prev = base;
    base->next = n;

    if(list->end == base) {
        list->end = n;
    }

    list->len += 1;
}

void linkedlist_insert_before(llist *list, node *base, const void *data) {
    node *n = newnode(list, data);

    n->next = base;
    n->prev = base->prev;
    base->prev = n;

    if(list->start == base) {
        list->start = n;
    }

    list->len += 1;
}

void linkedlist_bypass(llist *list, node *del) {
    if(del->prev == NULL) {
        list->start = del->next;
    } else {
        del->prev->next = del->next;
    }

    if(del->next == NULL) {
        list->end = del->prev;
    } else {
        del->next->prev = del->prev;
    }

    list->len -= 1;
}

void linkedlist_free_node(llist *list, node *n) {
    free(n->data);
    free(n);
}

void linkedlist_delete(llist *list, node *del) {
    linkedlist_bypass(list, del);
    linkedlist_free_node(list, del);
}

void linkedlist_free(llist *list) {
    node *n = list->start;
    node *next;

    while(n != NULL) {
        next = n->next;
        linkedlist_free_node(list, n);
        n = next;
    }

    free(list);
}
