#ifndef _PARSE_G_H_
#define _PARSE_G_H_

#include "../lib/hdict.h"
#include "../lib/ubarray.h"
#include "../lib/bitarr.h"
#include "../lib/hset.h"

typedef struct rule *rule_p;

typedef struct grammer *grammer_t;

struct grammer {
    ubarray_t rules;
    hdict_t symbols;
};

struct rule {
    int left;
    int len;
    int *right;
};

grammer_t read_grammer (FILE *f, 
                        char **term, int term_c,
                        ubarray_t nterm, ubarray_t consttok);

barr_t get_nullable (grammer_t G, int nterm_c);

barr_t *get_first (grammer_t G, barr_t nullable,
                   int nterm_c, int terminals);
                   
barr_t *get_follows (grammer_t G, barr_t nullable,
                     barr_t *firsts, int nterm_c, int terminals);

#endif
