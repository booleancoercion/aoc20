#include "aoc20.h"
#include "hashmap.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TILE_SIDE 10
#define NUM_EDGES 4

typedef struct hashmap hashmap_t;

typedef struct tile {
    int id;
    bool info[TILE_SIDE][TILE_SIDE];
} tile_t;

typedef struct edge {
    bool edge[TILE_SIDE];
} edge_t;

static void parse(tile_t **tiles, int *tiles_len, hashmap_t *edges_map);
static void parse_tile(FILE *input, tile_t *tile);
static void get_edges(const tile_t *tile, edge_t edges[]);
static int count_tiles(FILE *file);

static int edge_compare(const void *a, const void *b, void *udata) {
    return memcmp(a, b, sizeof(edge_t));
}

static uint64_t edge_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    return hashmap_sip(item, sizeof(edge_t), seed0, seed1);
}

void day20() {
    printf("Day 20 - Part 1\n");

    hashmap_t *edges_map =
        hashmap_new(sizeof(edge_t), 0, 0, 0, edge_hash, edge_compare, NULL);

    tile_t *tiles;
    int tiles_len;

    parse(&tiles, &tiles_len, edges_map);
    printf("Parsing complete; %d tiles, %zu unique edges.\n", tiles_len,
           hashmap_count(edges_map));

    for(int i = 0; i < tiles_len; i++) {
        edge_t edges[NUM_EDGES];
        get_edges(&tiles[i], edges);

        int edge_count = 0;
        for(int j = 0; j < NUM_EDGES; j++) {
            if(hashmap_get(edges_map, &edges[j]) == NULL) {
                edge_count++;
            }
        }

        if(edge_count > 0) {
            printf("Tile ID %d has edges that don't match.\n", tiles[i].id);
        }
    }

    free(tiles);
    hashmap_free(edges_map);
}

static void parse(tile_t **tiles, int *tiles_len, hashmap_t *edges_map) {
    FILE *input = fopen("inputs/day20.txt", "r");

    *tiles_len = count_tiles(input);
    *tiles = calloc(*tiles_len, sizeof(tile_t));

    for(int i = 0; i < *tiles_len; i++) {
        tile_t *tile = &(*tiles)[i];
        parse_tile(input, tile);

        edge_t edges[NUM_EDGES];
        get_edges(tile, edges);
        for(int j = 0; j < NUM_EDGES; j++) {
            hashmap_set(edges_map, &edges[j]);
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
            if(c == EOF) {
                printf("Encountered EOF with parsing tile! Exiting.\n");
                exit(1);
            }

            char ch = c;
            tile->info[i][j] = (ch == '#');
        }
        fseek(input, 1, SEEK_CUR); // Because newline
    }

    fseek(input, 1, SEEK_CUR); // ending newline
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
