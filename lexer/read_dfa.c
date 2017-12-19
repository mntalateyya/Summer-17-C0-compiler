#include "read_dfa.h"
#define SP sizeof(void*)

dfa_t read_dfa (char *filename) {
    FILE *f = fopen(filename, "r");
    dfa_t D = malloc(sizeof(struct dfa));
    fscanf(f,"%d", &D->size);
    fscanf(f,"%d", &D->num_tags);
    D->tags = malloc(SP*(D->num_tags+1));
    for (int i=1; i<=D->num_tags; ++i) {
        D->tags[i] = malloc(16);
        fscanf(f, "%s", D->tags[i]);
        D->tags[i] = realloc(D->tags[i], strlen(D->tags[i])+1);
    }
    D->states = malloc(SP*D->size);
    for (int i=0; i<D->size; ++i) {
        D->states[i] = malloc(sizeof(struct dfa_state));
        dfa_st tmp = D->states[i];
        tmp->next = malloc(sizeof(int)*128);
        tmp->next[0] = -1;
        fscanf(f, "%d", &tmp->tag);
        for (int j=0; j<128; ++j) {
            fscanf(f, "%d", tmp->next+j);
        }
    }
    return D;
}
