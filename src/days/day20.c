#include "aoc20.h"
#include "assert.h"
#include "hashmap.h"
#include "vector.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TILE_SIDE 10
#define NUM_EDGES 4

typedef struct hashmap hashmap_t;
typedef struct vector vector_t;

typedef struct tile {
    int id;
    bool info[TILE_SIDE][TILE_SIDE];
} tile_t;

typedef struct edge {
    bool edge[TILE_SIDE];
    vector_t *tiles;
} edge_t;

typedef struct tilematrix {
    struct {
        tile_t *source;
        int rotation; // 0,1,2,3 - clockwise
        bool flipped;
    } * tiles;
    size_t side;
} tilematrix_t;

static void parse(tile_t **tiles, size_t *tiles_len, hashmap_t *edges_map);
static void parse_tile(FILE *input, tile_t *tile);
static void get_edges(const tile_t *tile, edge_t edges[]);
static edge_t flip_edge(edge_t edge);
static edge_t *get_with_flip(hashmap_t *edges_map, edge_t *edge);
static int count_tiles(FILE *file);
static bool edge_vector_freer(const void *edge, void *udata);
static long find_edge_tiles(hashmap_t *edges_map, const tile_t *tiles,
                            size_t tiles_len, vector_t **edge_tiles);

static int edge_compare(const void *a_void, const void *b_void, void *udata) {
    const edge_t *a = a_void;
    const edge_t *b = b_void;
    return memcmp(&a->edge, &b->edge, sizeof(a->edge));
}

static uint64_t edge_hash(const void *item_void, uint64_t seed0,
                          uint64_t seed1) {
    const edge_t *item = item_void;
    return hashmap_sip(&item->edge, sizeof(item->edge), seed0, seed1);
}

static bool print_edge_details(const void *edge_void, void *udata) {
    const edge_t *edge = edge_void;
    int *counter = udata;

    printf("Edge #%.3d corresponds to %zu tiles.\n", *counter,
           edge->tiles->length);
    *counter += 1;
    return true;
}

void day20() {
    puts("Day 20 - Part 1");

    hashmap_t *edges_map =
        hashmap_new(sizeof(edge_t), 0, 0, 0, edge_hash, edge_compare, NULL);
    assert(NULL != edges_map);

    tile_t *tiles;
    size_t tiles_len;

    parse(&tiles, &tiles_len, edges_map);
    printf("Parsing complete; %zu tiles, %zu unique edges.\n", tiles_len,
           hashmap_count(edges_map));

    vector_t *edge_tiles;
    long mult = find_edge_tiles(edges_map, tiles, tiles_len, &edge_tiles);

    printf("Corner tile multiplication: %ld\n", mult);

    int counter = 1;
    hashmap_scan(edges_map, print_edge_details, &counter);

    vector_free(edge_tiles);
    free(tiles);
    hashmap_scan(edges_map, edge_vector_freer, NULL);
    hashmap_free(edges_map);
}

static long find_edge_tiles(hashmap_t *edges_map, const tile_t *tiles,
                            size_t tiles_len, vector_t **edge_tiles) {
    long mult = 1;
    *edge_tiles = vector_init(sizeof(tile_t *));
    for(size_t i = 0; i < tiles_len; i++) {
        const tile_t *tile = &tiles[i];

        edge_t edges[NUM_EDGES];
        get_edges(tile, edges);

        int nonmatching = 0;
        for(int j = 0; j < NUM_EDGES; j++) {
            edge_t *found = get_with_flip(edges_map, &edges[j]);
            assert(NULL != found);

            if(found->tiles->length == 1)
            { // this tile is the only tile that has this edge
                nonmatching++;
            }
        }

        if(nonmatching > 0) {
            vector_push(*edge_tiles, &tile);
        }

        if(nonmatching == 2) {
            mult *= tile->id;
        }
    }

    return mult;
}

static void parse(tile_t **tiles, size_t *tiles_len, hashmap_t *edges_map) {
    FILE *input = fopen("inputs/day20.txt", "r");

    *tiles_len = count_tiles(input);
    *tiles = calloc(*tiles_len, sizeof(tile_t));

    for(int i = 0; i < *tiles_len; i++) {
        tile_t *tile = &(*tiles)[i];
        parse_tile(input, tile);

        edge_t edges[NUM_EDGES];
        get_edges(tile, edges);
        for(int j = 0; j < NUM_EDGES; j++) {
            edge_t *edge;
            if(NULL != (edge = get_with_flip(edges_map, &edges[j]))) {
                vector_push_unique(edge->tiles, &tile);
            } else {
                edge = &edges[j];
                edge->tiles = vector_init(sizeof(tile_t *));
                vector_push(edge->tiles, &tile);

                hashmap_set(edges_map, edge);
            }
        }
    }

    fclose(input);
}

// Assuming that the file cursor is at the position
// where the 'T' from "Tile" is
static void parse_tile(FILE *input, tile_t *tile) {
    fscanf(input, "Tile %d:\n", &(tile->id));
    for(int i = 0; i < TILE_SIDE; i++) {
        for(int j = 0; j < TILE_SIDE; j++) {
            int c = fgetc(input);
            assert(c != EOF);
            assert(c == '#' || c == '.');

            char ch = c;
            tile->info[i][j] = (ch == '#');
        }
        fseek(input, 1, SEEK_CUR); // Because newline
    }

    fseek(input, 1, SEEK_CUR); // ending newline
}

static edge_t flip_edge(edge_t edge) {
    edge_t new_edge;
    new_edge.tiles = edge.tiles;
    for(int i = 0; i < TILE_SIDE; i++) {
        new_edge.edge[i] = edge.edge[TILE_SIDE - i - 1];
    }

    return new_edge;
}

static edge_t *get_with_flip(hashmap_t *edges_map, edge_t *edge) {
    edge_t *gotten;
    if(NULL != (gotten = hashmap_get(edges_map, edge))) {
        return gotten;
    }

    edge_t flipped = flip_edge(*edge);
    return hashmap_get(edges_map, &flipped);
}

static void get_edges(const tile_t *tile, edge_t edges[]) {
    for(int i = 0; i < TILE_SIDE; i++) {
        edges[0].edge[i] = tile->info[0][i];
        edges[1].edge[i] = tile->info[i][TILE_SIDE - 1];
        edges[2].edge[i] = tile->info[TILE_SIDE - 1][TILE_SIDE - 1 - i];
        edges[3].edge[i] = tile->info[TILE_SIDE - 1 - i][0];
    }
}

static int count_tiles(FILE *file) {
    int counter = 0;

    int c;
    while((c = fgetc(file)) != EOF) {
        char ch = c;
        if(ch == 'T') {
            counter++;
        }
    }

    rewind(file);
    return counter;
}

static bool edge_vector_freer(const void *edge_void, void *udata) {
    const edge_t *edge = edge_void;

    if(NULL != edge->tiles) {
        vector_free(edge->tiles);
    }

    return true;
}
