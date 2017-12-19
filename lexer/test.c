#include <stdio.h>
#include <assert.h>
#include "nfa.h"
#include "nfa2dfa.h"
#include "../lib/stack.h"
/*
void print_starr (nfa_starr A) {
    printf("length: %lu\n", A->len);
    for (size_t i=0; i<A->len; ++i) {
        printf("\tstate %lu:\t%p\n", i, (void*)A->arr[i]);
    }
}*/
bool ensure_nullity (nfa_st St, unsigned char a, unsigned char b) {
    while (a<b) {
        if (St->letters[a]) return false;
        ++a;
    }
    return true;
} 

bool ensure_av (nfa_st St, unsigned char a, unsigned char b) {
    while (a<b) {
        if (!St->letters[a]) return false;
        ++a;
    }
    return true;
}

int main () {
#ifdef TEST_NFA
    FILE *f = fopen("test_string","r");
    char *exp = strip(f);
    nfa_t result = parse_nfa(exp);
    nfa_st *A = result->states->arr;
    printf("Number of states: %lu\n", result->states->len);
    printf("start num: %lu\n",result->start->len);
    printf("start states: ");
    for(size_t i=0; i<result->start->len; ++i) {
        printf("%lu, ", result->start->arr[i]->id);
    }
    printf("\n");
    assert(A[0]->letters['i']);
    assert(ensure_nullity(A[0], 0, 'i') && ensure_nullity(A[0], 'i'+1, 128));
    assert(A[3]->letters[0]);
    assert(ensure_nullity(A[3], 1, 128));
    assert(ensure_nullity(A[11], 0, 128));
    assert(ensure_nullity(A[12], 0, 65));
    assert(ensure_av(A[12], 65, 91));
    assert(ensure_nullity(A[12], 91, 95));
    assert(A[12]->letters[95]);
    assert(!(A[12]->letters[96]));
    assert(ensure_av(A[12], 97, 123));
    assert(ensure_nullity(A[12], 123, 128));
    assert(A[14]->letters[0]->len==2);
    printf("state 14 on epsilon -> (%lu, %lu)\n", A[14]->letters[0]->arr[0]->id, 
            A[14]->letters[0]->arr[1]->id);
    printf("state 23, on a -> ");
    for(size_t i=0; i<A[23]->letters['a']->len; ++i) {
        printf("%lu, ", A[23]->letters['a']->arr[i]->id);
    }
    printf("\n");
    nfa_starr arr = epsilon_close(result, A[0]);
    printf("state 0 on epsilon -> ");
    for(size_t i=0; i<arr->len; ++i) {
        printf("%lu, ", arr->arr[i]->id);
    }
    printf("\n");
    free_starr(arr);
    arr = epsilon_close(result, A[19]);
    printf("state 19 on epsilon -> ");
    for(size_t i=0; i<arr->len; ++i) {
        printf("%lu, ", arr->arr[i]->id);
    }
    printf("\n");
    assert(A[7]->letters['i']->len==1);
    assert(A[7]->letters['i']->arr[0]==A[8]);
    free_starr(arr);
    free(exp);
    free_nfa_t(result);
    fclose(f);
#endif

#ifdef TEST_DFA
     FILE *f = fopen("test_string","r");
     char *exp = strip(f);
     printf("expression: ---%s---\n", exp);
     nfa_t N = parse_nfa(exp);
     dfa_t D = build_dfa(N);
     dfa_free(D);
     free_nfa_t(N);
     free(exp);
     fclose(f); 
#endif
    return 0;
}


