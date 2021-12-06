#include "vector.h"
#include <stdlib.h>
#include <string.h>

#define VECTOR_DEFAULT_CAPACITY 16

typedef struct vector vector_t;

static void vector_make_room(vector_t *vec);

vector_t *vector_init(size_t item_size) {
    vector_t *vector = malloc(sizeof(*vector));
    vector->items = NULL;
    vector->capacity = 0;
    vector->length = 0;
    vector->item_size = item_size;

    return vector;
}

void *vector_push(struct vector *vec, const void *item) {
    if(vec->length == vec->capacity) { // there's no room
        vector_make_room(vec);
    }

    char *current_item = vec->items + (vec->item_size) * vec->length;

    memcpy(current_item, item, vec->item_size);
    vec->length++;

    return current_item;
}

void *vector_push_unique(struct vector *vec, const void *item) {
    void *found = vector_find(vec, item);

    if(NULL != found) {
        return found;
    } else {
        return vector_push(vec, item);
    }
}

void *vector_find(const struct vector *vec, const void *item) {
    char *current_item = vec->items;
    for(size_t i = 0; i < vec->length; i++) {
        if(0 == memcmp(current_item, item, vec->item_size)) {
            return current_item;
        }

        current_item += vec->item_size;
    }

    return NULL;
}

void vector_free(vector_t *vec) {
    if(!(NULL == vec->items || 0 == vec->capacity)) {
        free(vec->items);
    }

    free(vec);
}

static void vector_make_room(vector_t *vec) {
    if(vec->capacity == 0) {
        vec->capacity = VECTOR_DEFAULT_CAPACITY;
        vec->items = calloc(vec->capacity, vec->item_size);
    } else {
        vec->capacity *= 2;
        vec->items = realloc(vec->items, vec->capacity);
    }
}
