#include <stdlib.h>
#include "stack.h"
#include "../lib/contracts.h"

stack_t stack_new () {
    stack_t S = malloc(sizeof(stack));
    S->top = malloc(sizeof(node));
    S->top->next = NULL;
    S->bottom = S->top;
    return S;
}

bool stack_empty (stack_t S) {
    if (S->top == S->bottom) return true;
    return false; 
}

void push (stack_t S, void *data) {
    node_p tmp = malloc(sizeof(node));
    tmp->data = data;
    tmp->next = S->top;
    S->top = tmp;
}

void *pop (stack_t S) {
    REQUIRES(!stack_empty(S));
    node_p tmp = S->top;
    S->top = tmp->next;
    void* data = tmp->data;
    free(tmp);
    return data;
}

void* peek(stack_t S) {
    if (S->top == S->bottom) return NULL;
    return S->top->data;
}

void stack_free (stack_t S) {
    node_p tmp = S->top;
    while (tmp!=NULL) {
        node_p prev = tmp;
        tmp = tmp->next;
        free(prev);
    }
    free(S);
}
