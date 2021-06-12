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

struct span {
    char *base;
    size_t start;
    size_t end;
};

struct grm {
    struct span span;
    int rule;
    bool result;
};

static int grm_compare(const void *a_void, const void *b_void, void *udata) {
    const struct grm *a = a_void;
    const struct grm *b = b_void;

    if(a->rule < b->rule) {
        return -1;
    } else if(a->rule > b->rule) {
        return 1;
    }

    size_t len_a = a->span.end - a->span.start;
    size_t len_b = b->span.end - b->span.start;
    size_t minlen = (len_a < len_b) ? len_a : len_b;

    char *a_base = a->span.base;

    char *b_base = b->span.base;

    for(size_t i = 0; i < minlen; i++) {
        char ca = a_base[a->span.start + i];
        char cb = b_base[b->span.start + i];
        if(ca < cb) {
            return -1;
        } else if(ca > cb) {
            return 1;
        }
    }

    if(len_a < len_b) {
        return -1;
    } else if(len_a > len_b) {
        return 1;
    }

    return 0;
}

static uint64_t grm_hash(const void *item_void, uint64_t seed0,
                         uint64_t seed1) {
    const struct grm *item = item_void;
    size_t start = item->span.start;
    size_t end = item->span.end;
    char *base = item->span.base;
    return hashmap_sip(base + start, sizeof(char) * (end - start), seed0,
                       seed1) +
           hashmap_sip(&(item->rule), sizeof(int), seed0, seed1);
}

static void parse(struct rule **rules, size_t *rules_len, char ***msgs,
                  size_t *msgs_len);
static size_t lines_until_empty(FILE *file);
static struct rule parse_rule(const char *line, const regex_t *preg);
static bool matches_rule(const struct span span, int rulenum,
                         const struct rule *rules, hashmap *cache);
static bool matches_two_rules_sliding(struct span span, int rule1, int rule2,
                                      const struct rule *rules, hashmap *cache);

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

    hashmap *cache =
        hashmap_new(sizeof(struct grm), 0, 0, 0, grm_hash, grm_compare, NULL);

    int counter = 0;
    for(size_t i = 0; i < msgs_len; i++) {
        printf("\rMessages left: %zu ", msgs_len - i);
        char *msg = msgs[i];
        struct span span = {.base = msg, .start = 0, .end = strlen(msg)};

        if(matches_rule(span, 0, rules, cache)) {
            counter += 1;
        }
    }

    printf("\rValid messages: %d\n", counter);

    hashmap_free(cache);
    for(size_t i = 0; i < msgs_len; i++) {
        free(msgs[i]);
    }
    free(msgs);
    free(rules);
}

static bool matches_rule(const struct span span, int rulenum,
                         const struct rule *rules, hashmap *cache) {

    if(span.end - span.start < 1) {
        printf("Illegal span! Exiting");
        exit(1);
    }
    struct grm grm = {.span = span, .rule = rulenum};
    struct grm *lookup;
    if((lookup = hashmap_get(cache, &grm)) != NULL) {
        return lookup->result;
    } else {
        struct rule rule = rules[rulenum];

        if(rule.kind == BASIC) {
            // returning here straight away so as to not clog
            // the cache
            if(span.end - span.start != 1) {
                return false;
            } else {
                return span.base[span.start] == rule.basic.ch;
            }

        } else if(rule.kind == ONE) {
            grm.result = matches_rule(span, rule.one.rule, rules, cache);

        } else if(rule.kind == TWO) {
            grm.result = matches_two_rules_sliding(
                span, rule.two.rules[0], rule.two.rules[1], rules, cache);

        } else { // rule.kind == PIPED
            grm.result = false;
            for(int i = 0; i < 2; i++) {
                if(matches_two_rules_sliding(span, rule.piped.rules[i][0],
                                             rule.piped.rules[i][1], rules,
                                             cache)) {
                    grm.result = true;
                    break;
                }
            }
        }

        hashmap_set(cache, &grm);
        return grm.result;
    }
}

static bool matches_two_rules_sliding(struct span span, int rule1, int rule2,
                                      const struct rule *rules,
                                      hashmap *cache) {
    for(size_t cutoff = span.start; cutoff + 1 < span.end; cutoff++) {
        struct span span1 = {
            .base = span.base, .start = span.start, .end = cutoff + 1};
        struct span span2 = {
            .base = span.base, .start = cutoff + 1, .end = span.end};

        if(matches_rule(span1, rule1, rules, cache) &&
           matches_rule(span2, rule2, rules, cache)) {
            return true;
        }
    }

    return false;
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
