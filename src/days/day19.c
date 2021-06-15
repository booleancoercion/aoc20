#include "aoc20.h"
#include "dynarr.h"
#include "hashmap.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dynarr dynarr;
typedef struct hashmap hashmap;

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
typedef struct cached_result {
    int rulenum;
    struct span span;
    bool result;
} cached_result;

static int cached_compare(const void *a_void, const void *b_void, void *udata) {
    const cached_result *a = a_void;
    const cached_result *b = b_void;

    if(a->rulenum != b->rulenum) {
        return (a->rulenum > b->rulenum) - (a->rulenum < b->rulenum);
    }

    size_t alen = a->span.end - a->span.start;
    size_t blen = b->span.end - b->span.start;

    int res =
        strncmp(a->span.base + a->span.start, b->span.base + b->span.start,
                (alen < blen) ? alen : blen);

    if(res == 0) {
        if(alen < blen) {
            return -1;
        } else if(alen > blen) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return res;
    }
}

static uint64_t cached_hash(const void *vitem, uint64_t seed0, uint64_t seed1) {
    const cached_result *item = vitem;
    size_t len = item->span.end - item->span.start;

    return hashmap_sip(item->span.base + item->span.start, len * sizeof(char),
                       seed0, seed1);
}

static void parse(struct rule **rules, size_t *rules_len, char ***msgs,
                  size_t *msgs_len);
static size_t lines_until_empty(FILE *file);
static struct rule parse_rule(const char *line);
static bool matches_rule(const struct span span, int rulenum,
                         const struct rule *rules, hashmap *cachemap);
static int count_pipes(const char *str);
static void populate_rule_arr(dynarr *intarr, const char *line);

static int count_matching_rules(const struct rule *rules, size_t rules_len,
                                char **msgs, size_t msgs_len,
                                hashmap *cachemap) {
    int counter = 0;
    for(size_t i = 0; i < msgs_len; i++) {
        printf("\rMessages left: %zu ", msgs_len - i);
        char *msg = msgs[i];
        struct span span = {.base = msg, .start = 0, .end = strlen(msg)};

        if(matches_rule(span, 0, rules, cachemap)) {
            counter += 1;
        }
    }
    return counter;
}

static void free_rule(struct rule *rule) {
    if(rule->kind == COMPOUND) {
        dynarr *arrays = rule->compound.arrays;
        dynarr *intarrays = arrays->elems;

        for(int j = 0; j < arrays->len; j++) {
            dynarr *intarr = &intarrays[j];
            int *elems = intarr->elems;
            free(elems);
        }

        dynarr_free(arrays);
    }
}

void day19() {
    struct rule *rules = NULL;
    size_t rules_len = 0;
    char **msgs = NULL;
    size_t msgs_len = 0;
    parse(&rules, &rules_len, &msgs, &msgs_len);

    hashmap *cachemap = hashmap_new(sizeof(cached_result), 0, 0, 0, cached_hash,
                                    cached_compare, NULL);

    printf("Day 19 - Part 1\n");
    printf("\rValid messages: %d\n\n",
           count_matching_rules(rules, rules_len, msgs, msgs_len, cachemap));

    hashmap_clear(cachemap, false);
    free_rule(&rules[8]);
    free_rule(&rules[11]);
    rules[8] = parse_rule("8: 42 | 42 8\n");
    rules[11] = parse_rule("11: 42 31 | 42 11 31\n");
    printf("Day 19 - Part 2\n");
    printf("\rValid messages: %d\n",
           count_matching_rules(rules, rules_len, msgs, msgs_len, cachemap));

    for(size_t i = 0; i < msgs_len; i++) {
        free(msgs[i]);
    }
    free(msgs);
    for(size_t i = 0; i < rules_len; i++) {
        struct rule *rule = &rules[i];
        free_rule(rule);
    }
    free(rules);
    hashmap_free(cachemap);
}

static bool increment_counters(int *counters, const bool *constants,
                               int last_nonconst, int lenn) {
    int sum = 0;
    for(int i = 0; i < last_nonconst; i++) {
        sum += counters[i];
    }

    for(int i = 0; i <= last_nonconst; i++) {
        if(i == last_nonconst) {
            return false;
        }
        if(constants[i]) {
            continue;
        }
        if(sum + 1 > lenn) {
            sum -= counters[i];
            counters[i] = 0;
            continue;
        }

        counters[i] += 1;
        sum += 1;
        break;
    }

    counters[last_nonconst] = lenn - sum;
    return true;
}

// if constants != NULL, only pays attention to the continuous starting and
// ending constant rules. first/last_nonconst will only be used in this case.
static bool iterate_with_counters(struct span span, const int *arr,
                                  int *counters, bool *constants,
                                  int first_nonconst, int last_nonconst, int n,
                                  const struct rule *rules, hashmap *cachemap) {
    struct span newspan = {.base = span.base};
    int sum = 0;
    for(int i = 0; i < n; i++) {
        newspan.start = span.start + sum;
        sum += counters[i] + 1;
        newspan.end = span.start + sum;
        if(newspan.end > span.end) {
            printf("Invalid newspan, exiting.\n");
            exit(1);
        }

        if(constants != NULL &&
           (!constants[i] || (i >= first_nonconst && i <= last_nonconst))) {
            continue;
        }

        if(!matches_rule(newspan, arr[i], rules, cachemap)) {
            return false;
        }
    }

    return true;
}

static bool matches_rule_list(const struct span span, dynarr *intarr,
                              const struct rule *rules, hashmap *cachemap) {
    int *arr = intarr->elems;
    int n = intarr->len;
    if(n == 1) {
        return matches_rule(span, arr[0], rules, cachemap);
    } else {
        bool result;

        int *counters = calloc(n, sizeof(int));
        bool *constants = calloc(n, sizeof(bool));

        int first_nonconst = -1;
        int last_nonconst = -1;

        for(int i = 0; i < n; i++) {
            int ruleno = arr[i];
            counters[i] = 0;
            constants[i] = rules[ruleno].kind == BASIC;
            if(rules[ruleno].kind != BASIC) {
                last_nonconst = i;
                if(first_nonconst == -1) {
                    first_nonconst = i;
                }
            }
        }

        int len = span.end - span.start;
        if(last_nonconst != -1) {
            counters[last_nonconst] = len - n;
        } else if(len != n) {
            result = false;
            goto ending;
        }

        if(!iterate_with_counters(span, arr, counters, constants,
                                  first_nonconst, last_nonconst, n, rules,
                                  cachemap)) {
            result = false;
            goto ending;
        }

        while(true) {
            if(iterate_with_counters(span, arr, counters, NULL, 0, 0, n, rules,
                                     cachemap)) {
                result = true;
                break;
            }
            if(last_nonconst == -1 ||
               !increment_counters(counters, constants, last_nonconst,
                                   len - n)) {
                result = false;
                break;
            }
        }

    ending:
        free(counters);
        free(constants);
        return result;
    }
}

static bool matches_rule_int(const struct span span, int rulenum,
                             const struct rule *rules, hashmap *cachemap) {
    if(span.end - span.start < 1) {
        printf("Illegal span! (%zu->%zu) Exiting\n", span.start, span.end);
        exit(1);
    }

    struct rule rule = rules[rulenum];
    size_t len = span.end - span.start;

    if(rule.kind == BASIC) {
        return (len == 1) && (span.base[span.start] == rule.basic.ch);

    } else if(rule.kind == COMPOUND) {
        dynarr *arrays = rule.compound.arrays;
        dynarr *arrays_array = arrays->elems;
        for(int option = 0; option < arrays->len; option++) {
            dynarr *intarr = &arrays_array[option];
            if((len >= intarr->len) &&
               matches_rule_list(span, intarr, rules, cachemap)) {
                return true;
            }
        }

        return false;
    } else {
        printf("Encountered unknown rule kind! Exiting.\n");
        exit(1);
    }
}

static bool matches_rule(const struct span span, int rulenum,
                         const struct rule *rules, hashmap *cachemap) {

    cached_result res = {.rulenum = rulenum, .span = span};
    cached_result *get;
    bool result;
    if((get = hashmap_get(cachemap, &res)) != NULL) {
        result = get->result;
    } else {
        result = matches_rule_int(span, rulenum, rules, cachemap);
        res.result = result;
        hashmap_set(cachemap, &res);
    }

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
