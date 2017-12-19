#include <stdio.h>
#include <stdlib.h>
#include "nfa.h"
#include "nfa2dfa.h"

int main (int num, char**args) {
    if (num!=2) {
        fprintf(stderr, "Use: export_dfa filename\n");
        return(1);
    }
    char *filename = args[1];
    FILE *f = fopen(filename, "r");
    if (f)
    {
        char *exp = strip(f);
        fclose(f);
        nfa_t N = parse_nfa(exp);
        free(exp);
        dfa_t D = build_dfa(N);
        printf("NFA size: %d, DFA size: %d\n", N->states->len, D->size);
        free_nfa_t(N);
        printf("Output filename (and path): ");
        char output[256];
        scanf("%s",output);
        f = fopen(output, "w");
        fprintf(f, "%d %d\n", D->size, D->num_tags);
        for (int i=1; i<=D->num_tags; ++i) {
            fprintf(f,"%s ",D->tags[i]);
        } fprintf(f,"\n");
        for (int i=0; i<D->size; ++i) {
            int *tmp = D->states[i]->next;
            fprintf(f, "%d ",D->states[i]->tag);
            for (int j=0; j<128; ++j) {
                fprintf(f, "%d ", tmp[j]);
            } fprintf(f, "\n");
            
        }
        fclose(f);
        dfa_free(D);
    } else {
        fprintf(stderr, "canonot open file %s\n", filename);
        return(1);
    }
    return 0;
}
