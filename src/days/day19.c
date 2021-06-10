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

static void print_rule(struct rule rule) {
    switch(rule.kind) {
    case BASIC:
        printf("Rule(%d: \"%c\")\n", rule.num, rule.basic.ch);
        break;
    case ONE:
        printf("Rule(%d: %d)\n", rule.num, rule.one.rule);
        break;
    case TWO:
        printf("Rule(%d: %d %d)\n", rule.num, rule.two.rules[0],
               rule.two.rules[1]);
        break;
    case PIPED:
        printf("Rule(%d: %d %d | %d %d)\n", rule.num, rule.piped.rules[0][0],
               rule.piped.rules[0][1], rule.piped.rules[1][0],
               rule.piped.rules[1][1]);
        break;
    }
}

void day19() {
    printf("Day 19 - Part 1\n");

    struct rule *rules = NULL;
    size_t rules_len = 0;
    char **msgs = NULL;
    size_t msgs_len = 0;
    parse(&rules, &rules_len, &msgs, &msgs_len);

    for(int i = 0; i < rules_len; i++) {
        print_rule(rules[i]);
    }

    // collect messages
    // check each message, with memoization

    free(msgs);
    free(rules);
}

static void parse(struct rule **rules, size_t *rules_len, char ***msgs,
                  size_t *msgs_len) {
    regex_t regex;
    if(regcomp(&regex,
               "([0-9]+): \"?([0-9ab]+)\"? ?([0-9]+)? ?\\|? ?([0-9]+)? "
               "?([0-9]+)?",
               REG_EXTENDED) != 0) {
        printf("regcomp error! exiting\n");
        exit(1);
    }

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
    ([0-9]+): \"?([0-9ab]+)\"? ?([0-9]+)? ?\|? ?([0-9]+)? ?([0-9]+)?

    0: Whole match
    1: Rule no.
    2: (BASIC/ONE/TWO/PIPED) rule 1 / char
    3: (TWO/PIPED) rule 2
    4: (PIPED) rule 3
    5: (PIPED) rule 4
    */
    regmatch_t matches[6];
    int rc;
    if((rc = regexec(preg, line, 6, matches, 0)) != 0) {
        char buffer[100];
        regerror(rc, preg, buffer, 100);
        printf("regexec error! line=%serrbuffer=%s\n", line, buffer);
        exit(1);
    }

    struct rule rule;
    rule.num = atoi(&line[matches[1].rm_so]);

    // i'm sorry for this code
    if(matches[2].rm_so != -1 && matches[3].rm_so == -1) {
        if(line[matches[2].rm_eo] == '"') {
            rule.kind = BASIC;
            rule.basic.ch = line[matches[2].rm_so];
        } else {
            rule.kind = ONE;
            rule.one.rule = atoi(&line[matches[2].rm_so]);
        }
    } else if(matches[3].rm_so != -1 && matches[4].rm_so == -1) {
        rule.kind = TWO;
        rule.two.rules[0] = atoi(&line[matches[2].rm_so]);
        rule.two.rules[1] = atoi(&line[matches[3].rm_so]);
    } else {
        rule.kind = PIPED;
        rule.piped.rules[0][0] = atoi(&line[matches[2].rm_so]);
        rule.piped.rules[0][1] = atoi(&line[matches[3].rm_so]);
        rule.piped.rules[1][0] = atoi(&line[matches[4].rm_so]);
        rule.piped.rules[1][1] = atoi(&line[matches[5].rm_so]);
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
