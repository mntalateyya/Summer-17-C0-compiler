/*
 * Hash sets, implemented with separate chaining
 *
 * 15-122 Principles of Imperative Computation
 */

#include <stdlib.h>
#include <stdbool.h>
/************************/
/*** Client interface ***/
/************************/

typedef void* elem;
typedef bool elem_equiv_fn (elem x, elem y);
typedef void hset_elem_free_fn (void*);
typedef size_t elem_hash_fn (elem x);


/******************************/
/*** Library implementation ***/
/******************************/

typedef struct chain_node chain;
struct chain_node {
  elem data;
  chain* next;
};

typedef struct hset_header hset;
struct hset_header {
  int size;
  int capacity;               /* 0 < capacity */
  chain **table;             /* \length(table) == capacity */
  elem_equiv_fn *elem_equiv;
  elem_hash_fn *elem_hash;
};

// Client-side type
typedef struct hset_header* hset_t;

hset_t hset_new(size_t capacity, 
                elem_equiv_fn elem_equiv, 
                elem_hash_fn elem_hash)
  /*@requires capacity > 0; @*/
  /*@ensures \result != NULL; @*/ ;

bool hset_contains(hset_t H, elem x)
  /*@requires H != NULL; @*/ ;

void hset_add(hset_t H, elem x)
  /*@requires H != NULL; @*/
  /*@ensures hset_contains(H, x); @*/ ;
  
void hset_free(hset* H, hset_elem_free_fn elem_free)
    /*@requires H != NULL; @*/;
