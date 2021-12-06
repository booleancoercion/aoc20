#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

struct vector {
    char *items;
    size_t length;
    size_t capacity;
    size_t item_size;
};

struct vector *vector_init(size_t item_size);
void *vector_push(struct vector *vec, const void *item);
void *vector_push_unique(struct vector *vec, const void *item);
void *vector_find(const struct vector *vec, const void *item);
void vector_free(struct vector *vec);

#endif // VECTOR_H
