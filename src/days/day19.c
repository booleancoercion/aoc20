#include "aoc20.h"
#include "hashmap.h"

#include <regex.h> // comes with *nix
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct hashmap hashmap;

enum rulekind { BASIC, ONE, TWO, PIPED };

struct rule {
    int num;
    enum rulekind kind;
    union {
        struct {
            char ch;
        } basic;
        struct {
            int rule;
        } one;
        struct {
            int rules[2];
        } two;
        struct {
            int rules[2][2];
        } piped;
    };
};

static void parse(struct rule **rules, size_t *rules_len, char ***msgs,
                  size_t *msgs_len);
static size_t lines_until_empty(FILE *file);
static struct rule parse_rule(const char *line, const regex_t *preg);

void day19() {
    printf("Day 19 - Part 1\n");

    struct rule *rules = NULL;
    size_t rules_len = 0;
    char **msgs = NULL;
    size_t msgs_len = 0;
    parse(&rules, &rules_len, &msgs, &msgs_len);
    // collect messages
    // check each message, with memoization

    free(msgs);
    free(rules);
}

static void parse(struct rule **rules, size_t *rules_len, char ***msgs,
                  size_t *msgs_len) {
    regex_t regex;
    regcomp(&regex,
            "(\\d+): "
            "(?:\"(\\w)\"|(\\d+))(?:.*?(\\d+))?(?:.*?(\\d+))?(?:.*?(\\d+))?",
            REG_EXTENDED);

    FILE *input = fopen("inputs/day19.txt", "r");

    *rules_len = lines_until_empty(input);
    *rules = calloc(*rules_len, sizeof(struct rule));

    char *line = NULL;
    size_t size = 0;
    ssize_t len = 0;

    // this loop and the next one will terminate when they encounter
    // an empty line.
    while((len = getline(&line, &size, input)) > 1) {
        struct rule rule = parse_rule(line, &regex);
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
    regfree(&regex);
}

static struct rule parse_rule(const char *line, const regex_t *preg) {
    /*
    (\d+): (?:\"(\w)\"|(\d+))(?:.*?(\d+))?(?:.*?(\d+))?(?:.*?(\d+))?

    0: Whole match
    1: Rule no.
    2: (BASIC) character
    3: (ONE/TWO/PIPED) rule 1
    4: (TWO/PIPED) rule 2
    5: (PIPED) rule 3
    6: (PIPED) rule 4
    */
    regmatch_t matches[7];
    regexec(preg, line, 7, matches, 0);

    struct rule rule;
    rule.num = atoi(&line[matches[1].rm_so]);

    // i'm sorry for this code
    if(matches[2].rm_so != -1) {
        rule.kind = BASIC;
        rule.basic.ch = line[matches[2].rm_so];
    } else if(matches[3].rm_so != -1 && matches[4].rm_so == -1) {
        rule.kind = ONE;
        rule.one.rule = atoi(&line[matches[3].rm_so]);
    } else if(matches[4].rm_so != -1 && matches[5].rm_so == -1) {
        rule.kind = TWO;
        rule.two.rules[0] = atoi(&line[matches[3].rm_so]);
        rule.two.rules[1] = atoi(&line[matches[4].rm_so]);
    } else {
        rule.kind = PIPED;
        rule.piped.rules[0][0] = atoi(&line[matches[3].rm_so]);
        rule.piped.rules[0][1] = atoi(&line[matches[4].rm_so]);
        rule.piped.rules[1][0] = atoi(&line[matches[5].rm_so]);
        rule.piped.rules[1][1] = atoi(&line[matches[6].rm_so]);
    }

    return rule;
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
