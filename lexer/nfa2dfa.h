#include <stdlib.h>
#include <stdbool.h>
#include "nfa.h"
#include "../lib/bitarr.h"
#include "../lib/ubarray.h"

#ifndef _NFA2DFA_H_
#define _NFA2DFA_H_
typedef struct dfa_pstate *dfa_pst;
struct dfa_pstate {
    barr_t Set;
    dfa_pst *next;
    int tag;
    int id;
};

typedef struct dfa_p *dfa_pptr;
struct dfa_p {
    ubarray_t states;
};

typedef struct dfa_state *dfa_st;
struct dfa_state {
    int *next;
    int tag; //index into tags array
};

typedef struct dfa *dfa_t;
struct dfa {
    int size;
    dfa_st *states;
    char **tags;        //array of tag names from 1
    int num_tags;   // number of tags
};

nfa_starr epsilon_close (nfa_t N, nfa_st st);

dfa_t build_dfa (nfa_t N);

void dfa_pfree (dfa_pptr D);

void dfa_free (dfa_t D);
#endif

