#include "aoc20.h"
#include "hashmap.h"
#include <stdio.h>

typedef struct hashmap hashmap;

typedef union point3 {
    struct {
        int x;
        int y;
        int z;
    };

    int co[3];
} point3;

typedef struct minmax_info {
    point3 min;
    point3 max;
} minmax_info;

static void handle_input(hashmap *map, minmax_info *mm);
static void advance_simulation(hashmap *map, minmax_info *mm);

static int point3_compare(const void *a_void, const void *b_void, void *udata) {
    const point3 *a = a_void;
    const point3 *b = b_void;

    for(int i = 0; i < 2; i++) {
        int comp = (a->co[i] > b->co[i]) - (a->co[i] < b->co[i]);

        if(comp) {
            return comp;
        }
    }

    return 0;
}

static uint64_t point3_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    return hashmap_sip(item, sizeof(point3), seed0, seed1);
}

void day_17() {
    printf("Day 17\n\n");

    hashmap *simulation =
        hashmap_new(sizeof(point3), 0, 0, 0, point3_hash, point3_compare, NULL);

    minmax_info minmax = {{{0, 0, 0}}, {{0, 0, 0}}};
    handle_input(simulation, &minmax);

    for(int i = 0; i < 6; i++) {
        advance_simulation(simulation, &minmax);
    }

    hashmap_free(simulation);
}

static bool copy_iter(const void *item, void *udata) {
    hashmap *copy = udata;

    // the cast is fine, because there is no modification
    hashmap_set(copy, (void *)item);

    return true;
}

static void advance_simulation(hashmap *map, minmax_info *mm) {
    hashmap *copy =
        hashmap_new(sizeof(point3), 0, 0, 0, point3_hash, point3_compare, NULL);

    hashmap_scan(map, copy_iter, copy);

    for(int x = mm->min.x - 1; x <= mm->max.x + 1; x++) {
        for(int y = mm->min.y - 1; y <= mm->max.y + 1; y++) {
            for(int z = mm->min.z - 1; z <= mm->max.z + 1; z++) {
                // check if (x, y, z) needs to be on or off
                // update (x, y, z) and possibly minmax bounds
            }
        }
    }

    hashmap_free(copy);
}

static void handle_input(hashmap *map, minmax_info *mm) {
    FILE *input = fopen("inputs/day17.txt", "r");
    if(input == NULL) {
        perror("Error opening day17.txt");
        exit(1);
    }

    int c, i = 0, j = 0;
    while((c = fgetc(input)) != EOF) {
        char ch = (char)c;
        if(ch == '\n') {
            i = 0;
            j++;
        } else {
            if(ch == '#') {
                point3 p = {.x = i, .y = j, .z = 0};
                hashmap_set(map, &p);

                if(mm->max.x < i) {
                    mm->max.x = i;
                }
                if(mm->max.y < j) {
                    mm->max.y = j;
                }
            }
            i++;
        }
    }
}
