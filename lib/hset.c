/*
 * Hash sets, implemented with separate chaining
 *
 * 15-122 Principles of Imperative Computation
 */

#include "hset.h"

bool is_hset(hset* H) {
  return  H != NULL
    && H->capacity > 0
    && H->size >= 0;
}

size_t elemhash(hset* H, elem x)
//@requires is_hset(H);
//@ensures 0 <= \result && \result < H->capacity;
{
  return abs(H->elem_hash(x) % H->capacity);
}

hset_t hset_new(size_t capacity, 
                elem_equiv_fn elem_equiv, 
                elem_hash_fn elem_hash)
//@requires capacity > 0;
//@ensures is_hset(\result);
{
  hset* H = malloc(sizeof(hset));
  H->size = 0;
  H->capacity = capacity;
  H->table = calloc(sizeof(chain*), capacity);
  H->elem_equiv = elem_equiv;
  H->elem_hash = elem_hash;
  return H;
}

bool hset_contains(hset* H, elem x)
//@requires is_hset(H);
{
  size_t i = elemhash(H, x);
  for (chain* p = H->table[i]; p != NULL; p = p->next) {
    if (H->elem_equiv(p->data, x)) {
      return true;
    }
  }

  return false;
}

void hset_add(hset* H, elem x)
//@requires is_hset(H);
//@ensures is_hset(H);
//@ensures hset_contains(H, x);
{
  size_t i = elemhash(H, x);
  for (chain* p = H->table[i]; p != NULL; p = p->next) {
    if (H->elem_equiv(p->data, x)) {
      p->data = x;
      return;
    }
  }

  // prepend new element
  chain* p = malloc(sizeof(chain));
  p->data = x;
  p->next = H->table[i];
  H->table[i] = p;
  (H->size)++;
}

void hset_free(hset* H, hset_elem_free_fn elem_free) 
//@requires is_hset(H);
{
    for (int i=0; i<H->capacity; ++i) {
        for (chain* p = H->table[i]; p != NULL; p = p->next) {
            if (elem_free) free(p->data);
            free(p);
        }
    }
    free(H->table);
    free(H);
}
