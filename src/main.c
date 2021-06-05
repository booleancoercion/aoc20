#include "aoc20.h"

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
    case 17:
        day17();
        break;
    default:
    case 18:
        day18();
        break;
    }
}
