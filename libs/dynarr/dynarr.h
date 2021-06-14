#ifndef DYNARR_H
#define DYNARR_H

#include <stdlib.h>

struct dynarr {
    void *elems;
    size_t elem_size;
    size_t len;
};

struct dynarr *dynarr_new(size_t len, size_t elem_size);
void dynarr_init(struct dynarr *arr, size_t len, size_t elem_size);
void dynarr_free(struct dynarr *arr);

#endif
