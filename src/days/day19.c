#include "aoc20.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct rule {
    int num;
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

void day19() {
    printf("Day 19 - Part 1\n");

    struct rule *rules = NULL;
    size_t rules_len = 0;
    char **msgs = NULL;
    size_t msgs_len = 0;
    parse(&rules, &rules_len, &msgs, &msgs_len);

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

static struct rule parse_rule(const char *line) {}

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
