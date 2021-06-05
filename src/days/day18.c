#include "aoc20.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum op { NONE, ADD, MUL };

static long compute(const char *str, size_t len);
static void char_error(const char *str, size_t len, size_t idx);
static bool performop(long *acc, enum op *nextop, long val);
static size_t find_next_matching_paren(const char *str, size_t idx);

void day18() {
    printf("Day 18 - Part 1\n");

    FILE *input = fopen("inputs/day18.txt", "r");
    if(input == NULL) {
        perror("Error opening day18.txt");
        exit(1);
    }

    long sum = 0;

    char *line;
    size_t size;
    ssize_t
        len; // signed because otherwise this will underflow when we're done :)

    while((len = getline(&line, &size, input)) > 1) { // >1 because newline
        sum += compute(line, len - 1);                // remove the newline
    }

    printf("The sum of all expressions is %ld\n", sum);

    free(line);
}

static long compute(const char *str, size_t len) {
    long acc = -1;

    enum op nextop = NONE;

    size_t idx = 0;
    while(idx < len) {
        char ch = str[idx];

        if(isdigit(ch)) {
            int digit = ch - '0';
            if(!performop(&acc, &nextop, digit)) {
                char_error(str, len, idx);
            }
        } else if(ch == '+' || ch == '*') {
            if(nextop != NONE) {
                char_error(str, len, idx);
            }

            if(ch == '+') {
                nextop = ADD;
            } else {
                nextop = MUL;
            }
        } else if(ch == '(') {
            size_t next_paren = find_next_matching_paren(str, idx);

            // the new length = old length, minus how many chars we've seen so
            // far (idx+1) added to how many chars are cut off from the end
            // (len-next_paren)
            //
            // to sum up, we get len - (idx + 1 + len - next_paren)
            // = next_paren - idx - 1
            size_t newlen = next_paren - idx - 1;
            long val = compute(str + idx + 1, newlen);
            if(!performop(&acc, &nextop, val)) {
                char_error(str, len, idx);
            }

            idx = next_paren + 1;
            continue;
        } else if(!isspace(ch)) {
            char_error(str, len, idx);
        }

        idx += 1;
    }

    return acc;
}

static bool performop(long *acc, enum op *nextop, long val) {
    if(*acc < 0) {
        if(*nextop != NONE) {
            return false;
        }

        *acc = val;
    } else {
        switch(*nextop) {
        case ADD:
            *acc += val;
            break;
        case MUL:
            *acc *= val;
            break;
        default:
            return false;
        }

        *nextop = NONE;
    }

    return true;
}

static size_t find_next_matching_paren(const char *str, size_t idx) {
    int balance_counter = 1;

    // adding one because we know str[idx] is the opening paren
    size_t i = idx + 1;
    char ch;
    while((ch = str[i]) != '\0') {
        if(ch == '(') {
            balance_counter += 1;
        } else if(ch == ')') {
            balance_counter -= 1;

            if(balance_counter == 0) {
                return i;
            }
        }

        i += 1;
    }

    return 0; // not found
}

static void char_error(const char *str, size_t len, size_t idx) {
    char ch = str[idx];

    printf("Error: encountered invalid character while computing the "
           "following expression: [%.*s] (ch=%c, idx=%zu)\n",
           (int)len, str, ch, idx);

    exit(1);
}
