#include "nfa2dfa.h"
#include "nfa.h"

#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_
struct tokens_table {
    char **tokens;
    int *tags;
    int len;
};
typedef struct tokens_table *toktable_t;

toktable_t tokenize (char *str, char *name, dfa_t D);

void toktable_free (toktable_t T);

#endif
