#include <stdlib.h>
#include <ctype.h>
#include "tokenize.h"

#define SP sizeof(void*)

int space_n (char c, int *n) {
    switch(c) {
        case ' ': return 1;
        case '\t': return 1;
        case '\f': return 1;
        case '\r': return 1;
        case '\v': return 1;
        case '\n': ++(*n); return 1;
        default: return 0;
    }
    return 0;
}

void line_lim (char *str, int index, int *from, int *to) {
    *from = index;
    while (*from!=0 && str[*from-1]!='\n') --(*from);
    *to = index;
    while (str[*to] && str[*to]!='\n') ++(*to);
}
void register_token (char *str, int start, int end, int *ids,
    dfa_st lstate, ubarray_t tokens, ubarray_t tags)
{
    char *token = malloc(end-start+1);
    strncpy(token, str+start, end-start);
    token[end-start] = '\0';
    printf("start: %d, end: %d, token: \"%s\"\n", start, end, token);
    array_add(tokens, token);
    array_add(tags, ids+lstate->tag);
}

toktable_t tokenize (unsigned char *str, char *name, dfa_t D) {
    if (!str || !name || !D) return NULL;
    
    ubarray_t tokens = array_new(16);
    ubarray_t tags = array_new(16);
    int ids[D->num_tags];
    for (int j=0; j<D->num_tags; ++j) ids[j] = j;
    
    int i=0, start=0, end=0, line=1;
    dfa_st state=D->states[0], lstate=NULL;
    while (str[i] && space_n(str[i], &line)) ++i;
    start = i;
    while (str[i]) {        
        if (state->next[str[i]]!=-1) {
            state = D[state->next[str[i]];
            if (state->tag) {
                end = i+1;
                lstate = state;
            }
            if (str[i]=='\n') ++line;
            ++i;
        } else if (end>start) {
            register_token(str, start, end, ids, lstate, tokens, tags);
            state=D->states[0];
            while (str[i] && space_n(str[i], &line)) ++i;
            start = i;
        } else {
            int *from = malloc(sizeof(int));
            int *to = malloc(sizeof(int));
            line_lim(str, i, from, to);
            fprintf(stderr, "%s:%d:%d: error: invalid token\n", 
                    name, line, i);
            fprintf(stderr, "%.*s\n", (int)(*to-*from), str+*from );
            fprintf(stderr, "%*s\n",(int)i+1, "^");
            exit(1);
        }
    }
    if (start!=i && end>start) {
        register_token(str, start, end, ids, lstate, tokens, tags);
    } else if (start!=i) {
        int *from = malloc(sizeof(int));
        int *to = malloc(sizeof(int));
        line_lim(str, i, from, to);
        fprintf(stderr, "%s:%d:%d: error: invalid token\n", 
                name, line, i);
        fprintf(stderr, "%.*s\n", (int)(*to-*from), str+*from );
        fprintf(stderr, "%*s\n",(int)i+1, "^");
        exit(1);
    }
    toktable_t T = malloc(sizeof(struct tokens_table));
    T->tokens = malloc(SP*array_len(tokens));
    T->tags = malloc(SP*array_len(tags));
    
    T->len = populate(tokens,  T->tokens);
    for (int j=0; j<T->len; ++j) {
        T->tags[j] = *(int*)array_get(tags, j);
    }
    array_free(tokens, NULL);
    array_free(tags, NULL);
    return T;
}

void toktable_free (toktable_t T) {
    for (int i=0; i<T->len; ++i) {
        free(T->tokens[i]);
    }
    free(T->tokens);
    free(T->tags);
    free(T);
}
