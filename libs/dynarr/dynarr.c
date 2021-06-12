#include "dynarr.h"

typedef struct dynarr dynarr;

dynarr *dynarr_init(size_t len, size_t elem_size) {
    dynarr *arr = malloc(sizeof(dynarr));

    arr->elem_size = elem_size;
    arr->len = len;
    arr->elems = calloc(len, elem_size);

    return arr;
}

void dynarr_free(dynarr *arr) {
    free(arr->elems);
    free(arr);
}
