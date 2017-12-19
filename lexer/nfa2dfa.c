#include "nfa.h"
#include "nfa2dfa.h"
#include "../lib/bitarr.h"
#include "../lib/stack.h"
#include "../lib/hdict.h"

#define SP sizeof(void*)

#ifdef TEST_DFA
void print_barr (barr_t B) {
    for (int i=0; i<B->len; ++i) {
        printf("%u ", barr_get(B, i));
    }
    printf("\n");
}
#endif

nfa_starr epsilon_close (nfa_t N, nfa_st st) {
    barr_t marks = barr_new(N->states->len);
    stack_t S = stack_new();
    barr_set(marks, st->id);
    push(S, st);
    
    //DFS on epsilon transitions
    while (!stack_empty(S)){
        nfa_st tmp = pop(S);
        if (tmp->letters[0]) {
            for (int i=0; i<tmp->letters[0]->len; ++i) {
                if (!barr_get(marks, tmp->letters[0]->arr[i]->id)) {
                    barr_set(marks, tmp->letters[0]->arr[i]->id);
                    push(S, tmp->letters[0]->arr[i]);
                }
            }
        }
    }
    
    //build nfa_starr from bitarray
    nfa_starr A = malloc(sizeof(struct state_arr));
    A->len = barr_countset(marks);
    A->arr = malloc(SP*A->len);
    int count = 0;
    for (int i=0; i<N->states->len; ++i) {
        if (barr_get(marks, N->states->arr[i]->id))
            A->arr[count++] = N->states->arr[i];
    }
    barr_free(marks);
    stack_free(S);  
    return A;
}

//given NFA states set, find the set of states on given transition
barr_t build_next (nfa_t N, barr_t source, nfa_starr *close, unsigned char c) {
    int n = N->states->len;
    barr_t B = barr_new(n);
    for (int i=0; i<n; ++i) {
        // for every state in set and if transition on given char is defined
        if (barr_get(source, i) && N->states->arr[i]->letters[c]) {
            //for every state in transition
            int m = N->states->arr[i]->letters[c]->len;
            for (int j=0; j<m; ++j) {
                // p is id of a next states
                int p = N->states->arr[i]->letters[c]->arr[j]->id;
                //if not already added, add its epsilon-close
                if (!barr_get(B, p)) {
                    for (int k=0; k<close[p]->len; ++k) {
                        barr_set(B, close[p]->arr[k]->id);
                    }
                }
            }
        }
    }
    return B;
}

// hash function for bitarrays
size_t hash_bitarray (void *ptr) {
    barr_t B = (barr_t) ptr;
    size_t hash = 1327294621657427659ul;
    for (int i=0; (size_t)i<B->len; ++i) {
        hash = hash * 1664525 + 1013904223*(int)barr_get(B, i);
    }
    return hash;
}

// wrapper for barr_free with void* input
void barr_freewrap (void *ptr) {
    barr_free((barr_t) ptr);
}

// wrapper for barr_equal with void* input
bool barr_equalwrap (void *ptr1, void *ptr2) {
    return barr_equal((barr_t) ptr1, (barr_t) ptr2);
}


// build DFA from NFA using subset construction
dfa_t build_dfa (nfa_t N) {
    stack_t S = stack_new(); //stack of states to be processed
    //dictionary of states with bitarrays as keys representing subset
    hdict_t Dict = hdict_new(32, barr_equalwrap, hash_bitarray, NULL);
    int m, p, q; //some temporary variables names
    int n = N->states->len; //NFA size
    nfa_starr *close = malloc(SP*n); //array of epsilon close for each state
    for (int i=0; i<n; ++i) {
        close[i] = epsilon_close(N, N->states->arr[i]);
    }
    // create partial-DFA and start state
    dfa_pptr DFA_p = malloc(sizeof(struct dfa_p));
    DFA_p->states = array_new(64);
    dfa_pst state = malloc(sizeof(struct dfa_pstate));
    state->Set = barr_new(n);
    state->next = calloc(128, SP);
    state->tag = 0;
    assert(array_len(DFA_p->states)==0);
    state->id = 0;
    array_add(DFA_p->states, state); //add to array of sets
    m = N->start->len;
    /* for each state s in NFA->start, if s is not in Set,
    add e-close of s to S */
    for (int i=0; i<m; ++i) {
        if (!barr_get(state->Set, N->start->arr[i]->id)) {
            q = N->start->arr[i]->id;
            p = close[q]->len;
            for (int j=0; j<p; ++j) {
                barr_set(state->Set, close[q]->arr[j]->id);
            }
        }
    }
    hdict_insert(Dict, state->Set, state);
    push(S, state);
    while (!stack_empty(S)) {
        state = pop(S);
        //build next for each character;
        for (unsigned char c=1; c<128; ++c) {
            barr_t B = build_next(N, state->Set, close, c);
            if (barr_countset(B)) { //if next is not empty set
                dfa_pst tmp = (dfa_pst)hdict_lookup(Dict, B);
                if (tmp) {
                    state->next[c] = tmp;
                    barr_free(B);
                } else {
                    tmp = malloc(sizeof(struct dfa_pstate));
                    tmp->Set = B;
                    tmp->next = calloc(128, SP);
                    tmp->id = array_len(DFA_p->states);
                    tmp->tag=0;
                    int i=0;
                    while (i<n && !tmp->tag) {
                        if (barr_get(B, i) && N->states->arr[i]->type) {
                            tmp->tag = N->states->arr[i]->type;
                        }
                        ++i;
                    }
                    array_add(DFA_p->states, tmp);
                    state->next[c] = tmp;
                    push(S, tmp);
                    hdict_insert(Dict, B, tmp);
                }
            } else {
                barr_free(B);
            }
        }
    }
    
    dfa_t DFA = malloc(sizeof(struct dfa));
    int size = array_len(DFA_p->states); 
    DFA->size = size;
    DFA->states = malloc(SP*size);
    for (int i=0; i<size; ++i) {
        DFA->states[i] =  malloc(sizeof(struct dfa_state));
        DFA->states[i]->next = calloc(128, sizeof(int));
        memset(DFA->states[i]->next, -1, 128*sizeof(int));
        dfa_pst tmp = array_get(DFA_p->states, i);
        DFA->states[i]->tag = tmp->tag;
    }
#ifdef TEST_DFA
    printf("size: %d\n", size);
#endif
    for (int i=0; i<size; ++i) {
#ifdef TEST_DFA
        printf("state: %d, accept: %u", i, DFA->states[i]->tag!=0);
        if (DFA->states[i]->tag)
            printf(", tag: %s", N->tags[DFA->states[i]->tag]);
        printf("\n");
        print_barr(((dfa_pst)array_get(DFA_p->states, i)) ->Set);
#endif
        dfa_pst tmp = array_get(DFA_p->states, i);
        for (unsigned char c=1; c<128; ++c) {
            if (tmp->next[c]) {
#ifdef TEST_DFA
                printf("state[%d]->next[%c:%u] = %d\n", i, c, c, tmp->next[c]->id);
#endif
                DFA->states[i]->next[c] = tmp->next[c]->id;
            }
        }
    }
#ifdef TEST_DFA
    barr_t B = build_next(N, ((dfa_pst)array_get(DFA_p->states, 7))->Set, close, 'i');
    print_barr(B);
    print_barr(((dfa_pst)array_get(DFA_p->states, 2))->Set);
    printf("%u\n", barr_equal(B, ((dfa_pst)array_get(DFA_p->states, 2))->Set)) ;
    barr_free(B);
#endif
    DFA->tags = N->tags;
    DFA->num_tags = N->num_tags;
    N->own_tags = false;
    for (int i=0; i<n; ++i) free_starr(close[i]);
    free(close);
    hdict_free(Dict);
    stack_free(S);
    dfa_pfree(DFA_p);
    return DFA;
}

void dfa_pfree (dfa_pptr D) {
    int size = array_len(D->states);
    for (int i=0; i<size; ++i) {
        barr_free( ((dfa_pst)array_get(D->states, i)) ->Set);
        free( ((dfa_pst)array_get(D->states, i)) ->next);
        free( ((dfa_pst)array_get(D->states, i)) );
    }
    array_free(D->states, NULL);
    free(D);
}

void dfa_free (dfa_t D) {
    for (int i=0; i<D->size; ++i) {
        free(D->states[i]->next);
        free(D->states[i]);
    }
    for (int i=1; i<=D->num_tags; ++i) {
        free(D->tags[i]);
    }
    free(D->states);
    free(D->tags);
    free(D);
}
