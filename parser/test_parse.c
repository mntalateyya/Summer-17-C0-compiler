#include <stdio.h>
#include "../lexer/nfa2dfa.h"
#include "../lexer/read_dfa.h"
#include "parse_g.h"
int main(int num, char **args) {
    //dfa_t D = read_dfa("lexer/c0_tokens.dfa");
    //printf("DFA parsed. size: %d\n", D->size);
    if (num!=2) {
        fprintf(stderr, "use test_parse (filename)\n");
        exit(1);
    }
    FILE *f = fopen(args[1],"r");
    if (!f) {
        fprintf(stderr, "cannot open file: %s\n", args[1]);
        exit(1);
    }
    ubarray_t nterm = array_new(8);
    ubarray_t consttok = array_new(8);
    grammer_t G = read_grammer (f, NULL, 0, nterm, consttok);
    printf("index of nil: %d\n", *(int*)hdict_lookup(G->symbols, "nil"));
    printf("const tokens: %lu, non-terminals: %lu\n", consttok->len, nterm->len);
    barr_t nullable = get_nullable(G, nterm->len);
    for (unsigned int i=0; i<nterm->len; ++i) 
        printf("nterm: %s, nullable:%d\n", (char*)array_get(nterm, i),
                                        barr_get(nullable, i));
    barr_t *firsts = get_first(G, nullable, nterm->len, consttok->len+1);
    barr_t *follows = get_follows(G, nullable, firsts, 
                                  nterm->len, consttok->len+1);
    for (unsigned int i=0; i<nterm->len; ++i) {
        printf("nterm: %s, FIRST set: (size: %lu)\n\t", (char*)array_get(nterm, i), barr_countset(firsts[i]));
        for (unsigned int j=1; j<=consttok->len; ++j) {
            if (barr_get(firsts[i], j))
                printf("%s, ", (char*)array_get(consttok, j-1));
        }
        printf("\n");
        printf("FOLLOWS set: (size: %lu)\n\t", barr_countset(follows[i]));
        /*for (int j=0; j<D->num_tags; ++j) {
            if (barr_get(firsts[i], j))
                printf("%s, ", D->tags[j]);
        }*/
        for (unsigned int j=1; j<=consttok->len; ++j) {
            if (barr_get(follows[i], j))
                printf("%s, ", (char*)array_get(consttok, j-1));
        }
        printf("\n");
    }
    
    return 0;
}
