#ifndef _UBARRAY_H_
#define _UBARRAY_H_
typedef void arr_elem_free_fn (void*);
typedef struct ubarray {
    void **arr;
    size_t len;
    size_t max;
} ubarray;

typedef struct ubarray *ubarray_t;

ubarray_t array_new (size_t len);

void* array_get (ubarray_t A, size_t index);

void array_set (ubarray_t A, size_t index, void *data);

void array_add (ubarray_t A, void *data);

size_t array_len (ubarray_t A);

size_t populate (ubarray_t A, void *B);

void array_free (ubarray *A, arr_elem_free_fn elem_free);

#endif
