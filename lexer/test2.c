#include "nfa2dfa.h"
#include "read_dfa.h"

int main() {
    dfa_t D = read_dfa("c0_tokens.dfa");
    FILE *f = fopen("c0_tokens_.dfa", "w");
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
    return 0;
}
