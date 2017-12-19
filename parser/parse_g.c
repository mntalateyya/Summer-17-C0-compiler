#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parse_g.h"

#define SP sizeof(void*)
#define MAX_TOKEN_LEN 16
#define MAX_CONST_COUNT 128

typedef struct constraint *fol_const;
struct constraint {
    int left;
    int right;
};

//valid non-terminal names can only have letters and ' (apostrophe) character
static int isnterm(char *str) {
    while (*str) {
        if ( !(isalpha(*str)) && !(*str=='\'') ) return 0;
        ++str;
    }
    return 1;
}

//return if 2 strings (passed as void*) are equal
static bool streq_wrap (void *p1, void *p2) {
    return !strcmp((char*)p1, (char*)p2);
}

//hash function for strings passed as void*
static size_t str_hash (void *p) {
    char *str = (char*)p;
    size_t hash = 17;
    while (*str) {
        hash = hash * 31 + *str;
        ++str;
    }
    return hash;
}

//parse a production rule from file f
//int_ket is array of pointers for use as hash table keys
//offset is the integer with which new symbols add to table
//consttok is an array of constant tokens
static rule_p read_rule (FILE *f, hdict_t H, int *intkey, int *offset,
                         ubarray_t consttok) {
    char * str = malloc(MAX_TOKEN_LEN);
    ubarray_t right = array_new(4); //store strings of right side
    fscanf(f, "%s", str);
    if (feof(f)) return NULL; //end of file reached, no more rules
    if (!hdict_lookup(H, str)) {
        //str is not a declared non-terminal
        fprintf(stderr, "Invalid left side of rule\n");
        exit(0);
    }
    rule_p R = malloc(sizeof(struct rule));
    R->left = *(int*)hdict_lookup(H, str); //int value of left
    
    // -> symbol
    str = malloc(3);
    fscanf(f, "%s", str);
    if (strcmp(str,"->")) {
        fprintf(stderr, "Expected -> for production rule\n");
        exit(0);
    } free(str);
    
    str = malloc(MAX_TOKEN_LEN);
    fscanf(f, "%s", str);
    while (!feof(f) && strcmp(str, ";")) {
    //loop until EOF reached or end of rule (; symbol)
        if (*str=='%') {
        //constant token
            array_add(right, malloc(strlen(str)));
            memcpy(array_get(right,right->len-1),str+1,strlen(str));
            free(str);
            array_add(consttok, array_get(right,right->len-1));
            if (!hdict_lookup(H, array_get(right,right->len-1))) {
            //add to consttok array
                hdict_insert(H, array_get(right,right->len-1),
                            intkey+(*offset)++);
            }                
        } else {
            array_add(right, str);
        }
        str = malloc(MAX_TOKEN_LEN);
        fscanf(f, "%s", str);
    } if (!strcmp(str, ";")) {
        free(str);
        R->len = array_len(right);
        R->right = malloc(sizeof(int)*R->len);
        for (int i=0; i<R->len; ++i) {  
            R->right[i] = *(int*)hdict_lookup(H, array_get(right,i));
        }
#ifdef DEBUG
        printf("%d -> ", R->left);
        for (int i=0; i<R->len; ++i) {
            printf("%d ", R->right[i]);
        }
        printf("\n");
#endif
        return R;
    } else {
        fprintf(stderr, "Unexpected end of input\n");
        exit(0);
    }
}

/*
keys:   0   1 ... nterm_c   nterm_c+1 ... netrm_c+term_c    .   ...
        nil --terminals--   --------non-terminals-------    $   constant tokens
*/
grammer_t read_grammer (FILE *f, char **term, int term_c,
                        ubarray_t nterm, ubarray_t consttok) {
    int nterm_c;
    fscanf(f, "%d", &nterm_c);
    hdict_t H = hdict_new(term_c/4+nterm_c/4+2,
                          streq_wrap, str_hash, NULL);
    char *str;
    int key_len = 1 + nterm_c+term_c+MAX_CONST_COUNT;
    int *intkey = malloc(sizeof(int)*(key_len));
    for (int i=0; i<key_len; ++i) intkey[i]=i;
    hdict_insert(H, "nil", intkey);
    for (int i=1; i<=nterm_c; ++i) {
        str = malloc(MAX_TOKEN_LEN);
        fscanf(f, "%s", str);
        if (!feof(f) && isnterm(str)) {    
            array_add(nterm, str);
            hdict_insert(H, str, intkey+i);
        } else if (feof(f)){
            fprintf(stderr, "Unexpected end of text\n");
            exit(1);
        } else {
            fprintf(stderr, "Invalid Non-terminal tag\n");
            exit(1);
        }
    }
    for (int i=1; i<=term_c; ++i) {
        hdict_insert(H, term[i], intkey+nterm_c+i);
    }
    hdict_insert(H, "$", intkey+nterm_c+term_c+1);
    int *offset = malloc(sizeof(int));
    *offset = nterm_c +  term_c + 2;
    grammer_t G = malloc(sizeof(struct grammer));
    G->rules = array_new(16);
    rule_p R = read_rule(f, H, intkey, offset, consttok);
    while (R) {
        array_add(G->rules, R);
        R = read_rule(f, H, intkey, offset, consttok);
    }
    G->symbols = H;
    return G;
}

static int nullable_helper (int* seq, int len, barr_t B) {
    if (len==0 || seq[0]==0) return 1;
    for (int i=0; i<len; ++i) {
        if (seq[i] > (int)B->len || !barr_get(B, seq[i]-1))
            return 0;
    } 
    return 1;
}

barr_t get_nullable (grammer_t G, int nterm_c) {
    barr_t B = barr_new(nterm_c);
    int change = 1;
    while (change) {
        change = 0;
        int lim = array_len(G->rules);
        for (int i=0; i<lim; ++i) {
            rule_p R = array_get(G->rules,i);
            if (nullable_helper(R->right, R->len, B) 
                && !barr_get(B, R->left-1)) 
            {
                barr_set(B, R->left-1);
                change = 1;
            }
        }
    }
    return B;
}


static int first_helper (rule_p R, barr_t *sets,
                         barr_t nullable, int nterm_c) 
{
    if (R->right[0]==0) return 0; //nil production
    int change = 0, i=0;
    do {
        if (R->right[i] > nterm_c) { //non-terminal
            // if not already in set, set and return 1
            if (!barr_get(sets[R->left-1], R->right[i]-nterm_c-1)) {
                barr_set(sets[R->left-1], R->right[i]-nterm_c-1);
                return 1;
            } return change;
        } else 
            change += barr_union(sets[R->left-1], sets[R->right[i]-1]);
        ++i;
    } while (i<R->len && barr_get(nullable, R->right[i-1]-1));
    return change;
}

//returns an array of bitarrays each corresponding to a non-terminal
//In the bitarray, each bit corresponds to a terminal
//array(3):bit(5) = 1 means 5th terminal is in FIRST(3rd non-terminal)
//order is the order as parsed (keys is hash table)
barr_t *get_first (grammer_t G, barr_t nullable,
                   int nterm_c, int terminals) 
{
    //terminsal = # terminal tags + # constant tokens
    barr_t *sets = malloc(SP*nterm_c);
    for (int i=0; i<nterm_c; ++i) 
        sets[i] = barr_new(terminals);
    int change = 1;
    int lim = array_len(G->rules);
    while (change) {
        change = 0;
        for (int i=0; i<lim; ++i) {
            change += first_helper(array_get(G->rules, i),
                                 sets, nullable, nterm_c);
        }
    }
    return sets;
}

// return 0 if constraint[left:right] already exist in consts, else return 1
static int constraint_helper (int left, int right, ubarray_t constraints) 
{
    if (right==left) return 0;
    for (unsigned int i=0; i<constraints->len; ++i) {
        fol_const tmp = (fol_const)constraints->arr[i];
        if (tmp->left==left && tmp->right==right) return 0;
    }
    return 1;
}

//given a rule gets the follows sets deduced from the rule;
static void get_constraints (rule_p R, barr_t nullable,
                      barr_t *firsts, barr_t *follows,
                      int nterm_c, ubarray_t constraints)
{
    /*  ---case: right = nil: exit
       |
        ---case: else:
                for each i in right let right = a i b:
                |    ---case: i is non-terminal:
                |   |   |   add First of b to follow of i
                |   |   |    ---case: naullable(b):
                |   |   |   |       add to constraints: (left C right)
                |   |   |    ---case: else:       
                |   |   |           next i
                |   |
                |    ---case: else:
                |           next i
    */
    if (R->right[0]==0) return;
    for (int i=0; i<R->len; ++i) {
        if (R->right[i] <= nterm_c) {
            if (i<R->len-1) {
                int j = i+1;
                do {
                    if (R->right[j] > nterm_c)
                        barr_set(follows[R->right[i]-1],
                                 R->right[j]-nterm_c-1);
                    else
                        barr_union(follows[R->right[i]-1], 
                                   firsts[R->right[j]-1]);
                    ++j;
                } while (j<R->len && R->right[j-1] <= nterm_c
                         && barr_get(nullable, R->right[j-1]-1));                 
            }
            if (nullable_helper(R->right+i+1, R->len-i-1, nullable)
                && constraint_helper(R->left, R->right[i], constraints)) {
                fol_const C = malloc(sizeof(struct constraint));
                C->right = R->right[i];
                C->left = R->left;
                array_add(constraints, C);
            }
        }
    }
}

/* returns follows set for each non-terminal as bitarrays where
each bitarray has as many bits as terminal
arrays(3):bit(5)==1 means 5th terminal is in follows(3rd non-terminal
order is the order of parsing (same as keys in hash table) */
barr_t *get_follows (grammer_t G, barr_t nullable,
                     barr_t *firsts, int nterm_c, int terminals)
{
    //create array of bitarrays
    barr_t *follows = malloc(SP*nterm_c);
    for (int i=0; i<nterm_c; ++i) 
        follows[i] = barr_new(terminals);
    //deduce constraints from rules
    int lim = array_len(G->rules);
    ubarray_t constraints = array_new(8);
    for (int i=0; i<lim; ++i) {
        get_constraints(array_get(G->rules, i), nullable,
                        firsts, follows, nterm_c, constraints);
    }
    //iterate until fixed point reached
    int change = 1;
    while (change) {
        change = 0;
        for (unsigned int i=0; i<constraints->len; ++i) {
            fol_const tmp = (fol_const)array_get(constraints, i);
            change += barr_union(follows[tmp->right-1], follows[tmp->left-1]);
        }
    }
    return follows;
}


