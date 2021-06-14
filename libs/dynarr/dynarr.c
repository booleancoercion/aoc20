#include "dynarr.h"

typedef struct dynarr dynarr;

dynarr *dynarr_new(size_t len, size_t elem_size) {
    dynarr *arr = malloc(sizeof(dynarr));
    dynarr_init(arr, len, elem_size);
    return arr;
}

void dynarr_init(struct dynarr *arr, size_t len, size_t elem_size) {
    arr->elem_size = elem_size;
    arr->len = len;
    arr->elems = calloc(len, elem_size);
}

void dynarr_free(dynarr *arr) {
    free(arr->elems);
    free(arr);
}
