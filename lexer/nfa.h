#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include "../lib/ubarray.h"
#include "../lib/bitarr.h"

#ifndef _NFA_H_
#define _NFA_H_
typedef enum {CONCAT,STAR,UNION} op_t;

struct regex_node {
    op_t op;
    struct regex_node *right;
    struct regex_node *left;
};

//
//linked list of struct state* elements
//
typedef struct state_arr *nfa_starr;

struct state_arr {
    struct state **arr;
    int len;
};

//
//struct of a single state
//
typedef struct state *nfa_st;

struct state {
    //each state has an array of nfa_starr
    //each corresponding to nexts of a single character
    nfa_starr *letters;
    uint16_t type;
    int id;
};

typedef struct nfa *nfa_p;
struct nfa {
    nfa_starr states;   //array of state pointers
    nfa_starr start;    //start states
    nfa_starr **end;    //end arrows
    int end_c;       //length of end
};

typedef struct nfa_parse *nfa_t;
struct nfa_parse {
    nfa_starr states;   //array of state pointers
    nfa_starr start;    //start states
    char **tags;        //array of tag names
    int num_tags;
    bool own_tags;
};

void free_starr (nfa_starr St);

nfa_p char2nfa (uint8_t chr, int type);

nfa_p concat (nfa_p N1, nfa_p N2);
    
nfa_p star (nfa_p N);

nfa_p union_ (nfa_p N1, nfa_p N2);

char *strip (FILE *f);

nfa_t parse_nfa (char *str);

void free_nfa_t (nfa_t N);

#endif
