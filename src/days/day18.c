#include "aoc20.h"
#include "linkedlist.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum op { ADD, MUL };

static void build_atomlist(const char *str, size_t len,
                           long (*compute)(const char *, size_t),
                           struct linkedlist *atomlist);
static bool evaluate_atoms(struct linkedlist *atomlist, bool multiplying);
static long compute_both(const char *str, size_t len, bool smart);
static long compute_naive(const char *str, size_t len);
static long compute_smart(const char *str, size_t len);
static size_t find_next_matching_paren(const char *str, size_t idx);
static void char_error(const char *str, size_t len, size_t idx);
static void eval_error(const char *str, size_t len);

void day18() {
    FILE *input = fopen("inputs/day18.txt", "r");
    if(input == NULL) {
        perror("Error opening day18.txt");
        exit(1);
    }

    long naive_sum = 0;
    long smart_sum = 0;

    char *line = NULL;
    size_t size = 0;
    // signed because otherwise this will underflow when we're done :)
    ssize_t len = 0;

    while((len = getline(&line, &size, input)) > 1) { // >1 because newline
        naive_sum += compute_naive(line, len - 1); // -1 to remove the newline
        smart_sum += compute_smart(line, len - 1);
    }

    printf("Day 18 - Part 1\n"
           "The sum of all expressions is %ld\n\n"
           "Day 18 - Part 2\n"
           "The sum of all expressions is %ld\n",
           naive_sum, smart_sum);

    free(line);
}

enum atomtype { OP, VALUE };

struct atom {
    enum atomtype type;
    union {
        enum op op;
        long value;
    };
};

static void build_atomlist(const char *str, size_t len,
                           long (*compute)(const char *, size_t),
                           struct linkedlist *atomlist) {
    size_t idx = 0;
    while(idx < len) {
        char ch = str[idx];

        if(isdigit(ch)) {
            long val = ch - '0';
            struct atom atom = {.type = VALUE, {.value = val}};
            linkedlist_insert_end(atomlist, &atom);
        } else if(ch == '+' || ch == '*') {
            enum op op = (ch == '+') ? ADD : MUL;
            struct atom atom = {.type = OP, {.op = op}};
            linkedlist_insert_end(atomlist, &atom);
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
            struct atom atom = {.type = VALUE, {.value = val}};
            linkedlist_insert_end(atomlist, &atom);

            idx = next_paren + 1;
            continue;
        } else if(!isspace(ch)) {
            char_error(str, len, idx);
        }

        idx += 1;
    }
}

static bool evaluate_atoms(struct linkedlist *atomlist, bool multiplying) {
    struct node *p = atomlist->start;

    while(p != NULL) {
        struct atom *data = p->data;
        if(data->type != OP || (data->op != ADD && !multiplying)) {
            p = p->next;
            continue;
        }

        if(p->prev == NULL || p->next == NULL) {
            return false;
        }

        struct atom *left = p->prev->data;
        struct atom *right = p->next->data;
        if(left->type != VALUE || right->type != VALUE) {
            return false;
        }

        data->type = VALUE;
        data->value = (data->op == ADD) ? left->value + right->value
                                        : left->value * right->value;
        linkedlist_delete(atomlist, p->prev);
        linkedlist_delete(atomlist, p->next);

        p = p->next;
    }

    return true;
}

static long compute_both(const char *str, size_t len, bool smart) {
    struct linkedlist *atomlist = linkedlist_init(sizeof(struct atom));
    build_atomlist(str, len, smart ? compute_smart : compute_naive, atomlist);

    // if smart is true, then we first do an evaluation of only additions.
    // in any case, this will produce an error if any of the performed
    // evaluations fails.
    if((smart && !evaluate_atoms(atomlist, false)) ||
       !evaluate_atoms(atomlist, true)) {
        eval_error(str, len);
    }

    // after evaluation, we should be left with a single node
    struct atom *atom = atomlist->start->data;
    if(atom->type != VALUE) {
        eval_error(str, len);
    }
    long result = atom->value;

    linkedlist_free(atomlist);

    return result;
}

static long compute_naive(const char *str, size_t len) {
    return compute_both(str, len, false);
}

static long compute_smart(const char *str, size_t len) {
    return compute_both(str, len, true);
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

static void eval_error(const char *str, size_t len) {
    printf("Error: encountered evaluation error while computing the "
           "following expression: [%.*s]\n",
           (int)len, str);

    exit(1);
}
