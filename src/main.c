#include "aoc20.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void solve_day(int day);

int main(int argc, char **argv) {
    if(argc < 2) {
        // no arguments given
        solve_day(0);
    } else {
        int day = atoi(argv[1]);
        solve_day(day);
    }
}

void solve_day(int day) {
    switch(day) {
    default:
    case 19:
        day19();
        break;
    case 18:
        day18();
        break;
    case 17:
        day17();
        break;
    }
}
