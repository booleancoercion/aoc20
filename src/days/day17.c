#include "aoc20.h"
#include "hashmap.h"
#include <stdio.h>

typedef struct hashmap hashmap;

typedef union point4 {
    struct {
        int x;
        int y;
        int z;
        int w;
    };

    int co[4];
} point4;

typedef struct minmax_info {
    point4 min;
    point4 max;
} minmax_info;

static void handle_input(hashmap *map, minmax_info *mm);
static void advance_simulation(hashmap *map, minmax_info *mm, bool is_4d);
static int count_neighbors(hashmap *map, const point4 *p, bool is_4d);
static void update_minmax(minmax_info *mm, const point4 *p, bool is_4d);
static void day17_doer(bool is_4d);

static int point4_compare(const void *a_void, const void *b_void, void *udata) {
    const point4 *a = a_void;
    const point4 *b = b_void;

    for(int i = 0; i < 4; i++) {
        int comp = (a->co[i] > b->co[i]) - (a->co[i] < b->co[i]);

        if(comp) {
            return comp;
        }
    }

    return 0;
}

static uint64_t point4_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    return hashmap_sip(item, sizeof(point4), seed0, seed1);
}

void day17() {
    day17_doer(false);
    printf("\n");
    day17_doer(true);
}

static void day17_doer(bool is_4d) {
    printf("Day 17 - Part %d\n", is_4d ? 2 : 1);

    hashmap *simulation =
        hashmap_new(sizeof(point4), 0, 0, 0, point4_hash, point4_compare, NULL);

    minmax_info minmax = {{{0, 0, 0, 0}}, {{0, 0, 0, 0}}};
    handle_input(simulation, &minmax);

    for(int i = 0; i < 6; i++) {
        advance_simulation(simulation, &minmax, is_4d);
    }

    printf("The number of active cells after 6 iterations: %zu\n",
           hashmap_count(simulation));

    hashmap_free(simulation);
}

static bool copy_iter(const void *item, void *udata) {
    hashmap *copy = udata;

    // the cast is fine, because there is no modification
    hashmap_set(copy, (void *)item);

    return true;
}

static void advance_simulation(hashmap *map, minmax_info *mm, bool is_4d) {
    hashmap *copy =
        hashmap_new(sizeof(point4), 0, 0, 0, point4_hash, point4_compare, NULL);

    hashmap_scan(map, copy_iter, copy);

    // the checks on min/max w are done to ensure that w's for loop
    // only has w=0.

    int minx = mm->min.x, miny = mm->min.y, minz = mm->min.z,
        minw = is_4d ? mm->min.w : 1;
    int maxx = mm->max.x, maxy = mm->max.y, maxz = mm->max.z,
        maxw = is_4d ? mm->max.w : -1;

    for(int x = minx - 1; x <= maxx + 1; x++) {
        for(int y = miny - 1; y <= maxy + 1; y++) {
            for(int z = minz - 1; z <= maxz + 1; z++) {
                for(int w = minw - 1; w <= maxw + 1; w++) {
                    // check if P needs to be on or off
                    // update P and possibly minmax bounds

                    point4 p = {.x = x, .y = y, .z = z, .w = w};
                    bool active = hashmap_get(copy, &p) != NULL;
                    int neighbors = count_neighbors(copy, &p, is_4d);

                    if(active && neighbors != 2 && neighbors != 3) {
                        hashmap_delete(map, &p);
                    } else if(!active && neighbors == 3) {
                        hashmap_set(map, &p);
                        update_minmax(mm, &p, is_4d);
                    }
                }
            }
        }
    }

    hashmap_free(copy);
}

static void update_minmax(minmax_info *mm, const point4 *p, bool is_4d) {
    int upper = is_4d ? 4 : 3;
    for(int i = 0; i < upper; i++) {
        if(p->co[i] < mm->min.co[i]) {
            mm->min.co[i] = p->co[i];
        }
        if(p->co[i] > mm->max.co[i]) {
            mm->max.co[i] = p->co[i];
        }
    }
}

static int count_neighbors(hashmap *map, const point4 *p, bool is_4d) {
    int x = p->x, y = p->y, z = p->z, w = p->w, counter = 0;

    for(int dx = -1; dx <= 1; dx++) {
        for(int dy = -1; dy <= 1; dy++) {
            for(int dz = -1; dz <= 1; dz++) {
                if(is_4d) {
                    for(int dw = -1; dw <= 1; dw++) {
                        if(dx == 0 && dy == 0 && dz == 0 && dw == 0) {
                            continue;
                        }

                        point4 temp = {
                            .x = x + dx, .y = y + dy, .z = z + dz, .w = w + dw};
                        if(hashmap_get(map, &temp) != NULL) {
                            counter += 1;
                        }
                    }
                } else {
                    if(dx == 0 && dy == 0 && dz == 0) {
                        continue;
                    }
                    point4 temp = {
                        .x = x + dx, .y = y + dy, .z = z + dz, .w = w};
                    if(hashmap_get(map, &temp) != NULL) {
                        counter += 1;
                    }
                }
            }
        }
    }

    return counter;
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
                point4 p = {.x = i, .y = j, .z = 0, .w = 0};
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
