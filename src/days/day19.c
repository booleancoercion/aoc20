#include "aoc20.h"
#include "dynarr.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dynarr dynarr;

enum rulekind { BASIC, COMPOUND };
struct rule {
    int num;
    enum rulekind kind;
    union {
        struct {
            char ch;
        } basic;
        struct {
            dynarr *arrays;
        } compound;
    };
};

struct span {
    char *base;
    size_t start;
    size_t end;
};

static void parse(struct rule **rules, size_t *rules_len, char ***msgs,
                  size_t *msgs_len);
static size_t lines_until_empty(FILE *file);
static struct rule parse_rule(const char *line);
static bool matches_rule(const struct span span, int rulenum,
                         const struct rule *rules);
static int count_pipes(const char *str);
static void populate_rule_arr(dynarr *intarr, const char *line);

static void print_rule(struct rule rule) {
    if(rule.kind == BASIC) {
        printf("Rule(%d: \"%c\")\n", rule.num, rule.basic.ch);
    } else if(rule.kind == COMPOUND) {
        printf("Rule(%d: ", rule.num);

        dynarr *arrays_array = rule.compound.arrays->elems;
        for(int i = 0; i < rule.compound.arrays->len; i++) {
            int *intarr = arrays_array[i].elems;
            for(int j = 0; j < arrays_array[i].len; j++) {
                if(j + 1 == arrays_array[i].len) {
                    printf("%d", intarr[j]);
                } else {
                    printf("%d ", intarr[j]);
                }
            }

            if(i + 1 == rule.compound.arrays->len) {
                printf(")\n");
            } else {
                printf(" | ");
            }
        }
    }
}

void day19() {
    printf("Day 19 - Part 1\n");

    struct rule *rules = NULL;
    size_t rules_len = 0;
    char **msgs = NULL;
    size_t msgs_len = 0;
    parse(&rules, &rules_len, &msgs, &msgs_len);

    for(size_t i = 0; i < rules_len; i++) {
        print_rule(rules[i]);
    }

    return;

    int counter = 0;
    for(size_t i = 0; i < msgs_len; i++) {
        // printf("\rMessages left: %zu ", msgs_len - i);
        char *msg = msgs[i];
        struct span span = {.base = msg, .start = 0, .end = strlen(msg)};

        if(matches_rule(span, 0, rules)) {
            counter += 1;
        }
        printf("\n");
    }

    printf("\rValid messages: %d\n", counter);

    for(size_t i = 0; i < msgs_len; i++) {
        free(msgs[i]);
    }
    free(msgs);
    free(rules);
}

static bool matches_rule_int(const struct span span, int rulenum,
                             const struct rule *rules) {
    if(span.end - span.start < 1) {
        printf("Illegal span! (%zu->%zu) Exiting\n", span.start, span.end);
        exit(1);
    }

    return false;
}

static void print_spaces(int level) {
    for(int i = 0; i < level; i++) {
        printf(" ");
    }
}

static bool matches_rule(const struct span span, int rulenum,
                         const struct rule *rules) {
    static int level = 0;

    print_spaces(level * 4);

    printf("%d: %.*s\n", rulenum, (int)(span.end - span.start),
           span.base + span.start);
    level += 1;
    bool result = matches_rule_int(span, rulenum, rules);
    level -= 1;
    print_spaces(level * 4);
    printf("result: %s\n", result ? "true" : "false");

    return result;
}

static void parse(struct rule **rules, size_t *rules_len, char ***msgs,
                  size_t *msgs_len) {
    FILE *input = fopen("inputs/day19.txt", "r");

    *rules_len = lines_until_empty(input);
    *rules = calloc(*rules_len, sizeof(struct rule));

    char *line = NULL;
    size_t size = 0;
    ssize_t len = 0;

    // this loop and the next one will terminate when they encounter
    // an empty line.
    while((len = getline(&line, &size, input)) > 1) {
        struct rule rule = parse_rule(line);
        (*rules)[rule.num] = rule;
    }

    *msgs_len = lines_until_empty(input);
    *msgs = calloc(*msgs_len, sizeof(char *));

    int i = 0;
    while((len = getline(&line, &size, input)) > 1) {
        // no -1 because len doesn't include the null terminator
        char *copy = calloc(len, sizeof(char));
        strncpy(copy, line, len); // this also copies the newline to the end
        copy[len - 1] = '\0';     // replace the ending newline with a null char

        (*msgs)[i] = copy;
        i += 1;
    }

    free(line);
    fclose(input);
}

static struct rule parse_rule(const char *line) {
    // the rule number is always the first number to appear in the line.
    int ruleno = atoi(line);

    line = strchr(line, ' ');
    if(line == NULL) {
        printf("Couldn't find space in rule! exiting\n");
        exit(1);
    }
    line += 1; // to start *after* the space

    struct rule rule = {.num = ruleno};
    if(line[0] == '"') {
        char ch = line[1];
        rule.kind = BASIC;
        rule.basic.ch = ch;
    } else {
        int pipes = count_pipes(line);
        dynarr *arrays = dynarr_new(pipes + 1, sizeof(dynarr));

        dynarr *elems = arrays->elems;
        for(int i = 0; i < arrays->len; i++) {
            populate_rule_arr(&elems[i], line);

            // next-next, because the next char is a a space.
            // no need to check for null, since it will only appear on the last
            // iteration anyway
            line = strchr(line, '|') + 2;
        }

        rule.kind = COMPOUND;
        rule.compound.arrays = arrays;
    }

    return rule;
}

static int count_pipes(const char *str) {
    char ch;
    int i = 0, counter = 0;
    while((ch = str[i]) != '\0') {
        if(ch == '|') {
            counter += 1;
        }

        i++;
    }

    return counter;
}

static void populate_rule_arr(dynarr *intarr, const char *line) {
    // this loop counts the spaces that appear between two numbers, until it
    // encounters a pipe or the end of the string.
    // i starts at 1 because we can't do line[i-1] at the start of the string.
    int i = 1, counter = 0;
    while(line[i + 1] != '|' && line[i + 1] != '\0') {
        if(line[i] == ' ' && isdigit(line[i - 1]) && isdigit(line[i + 1])) {
            counter += 1;
        }

        i++;
    }

    dynarr_init(intarr, counter + 1, sizeof(int));
    int *elems = intarr->elems;

    for(i = 0; i < intarr->len; i++) {
        elems[i] = atoi(line);

        // this will produce nullptr at the last iteration, but that's fine
        line = strchr(line, ' ') + 1;
    }
}

// Returns the number of lines from the current position in the file
// until an empty line is encountered (exclusive).
// This function also returns the file position to where it was
// before it was called.
static size_t lines_until_empty(FILE *file) {
    long curr = ftell(file);

    bool last_was_newline = false;
    size_t lines = 0;

    int out = 0;
    while((out = fgetc(file)) != EOF) {
        char ch = out;
        if(ch == '\n') {
            if(last_was_newline) {
                break;
            } else {
                last_was_newline = true;
            }
            lines += 1;
        } else {
            if(last_was_newline) {
                last_was_newline = false;
            }
        }
    }

    fseek(file, curr, SEEK_SET);
    return lines;
}
