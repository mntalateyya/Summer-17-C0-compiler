#include <stdbool.h>
#include "../lib/contracts.h"

#ifndef _STACK_H_
#define _STACK_H_

typedef struct stack {
    struct node *top;
    struct node *bottom;
} stack;

typedef struct stack *stack_t;

struct node {
    void *data;
    struct node *next;
} node;

typedef struct node *node_p;

stack_t stack_new ();

void push (stack_t S, void* data);

void *pop (stack_t S);

void *peek (stack_t S);

bool stack_empty(stack_t S);

void stack_free (stack_t S);

#endif
