#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ubarray.h"
#include "contracts.h"

#define SP sizeof(void*)

bool is_ubarray_t (ubarray *A) {
    return (A!=NULL && A->arr!=NULL 
            && A->max > A->len);
}

ubarray *array_new (size_t size) {
    REQUIRES(size>0);
    ubarray *result = malloc(sizeof(ubarray));
    result->arr = malloc(SP*size);
    result->len = 0;
    result->max = size;
    ENSURES(is_ubarray_t(result));
    return result;
}

void* array_get (ubarray *A, size_t index) {
    REQUIRES(is_ubarray_t(A));
    REQUIRES(index<A->len);
    return A->arr[index];
}

void array_set (ubarray *A, size_t index, void *data) {
    REQUIRES(is_ubarray_t(A));
    REQUIRES(index<A->len);
    A->arr[index] = data;
}

void resize_arr (ubarray *A) {
    REQUIRES(A!=NULL);
    assert(A->max<((size_t)1<<31));
    if (A->max==A->len) {
        A->arr = realloc(A->arr, SP*A->max*2);
        A->max *= 2;
    }
    REQUIRES(is_ubarray_t(A));
}

void array_add (ubarray *A, void *data) {
    REQUIRES(is_ubarray_t(A));
    A->arr[A->len++] = data;
    resize_arr(A);
    ENSURES(is_ubarray_t(A));
}

size_t array_len (ubarray *A) {
    REQUIRES(is_ubarray_t(A));
    return A->len;
}

size_t populate (ubarray_t A, void *B) {
    memcpy(B, A->arr, SP*A->len);
    return A->len;
}
void array_free (ubarray *A, arr_elem_free_fn elem_free) {
    REQUIRES(is_ubarray_t(A));
    if (elem_free) {
        for (size_t i=0; i<A->len; ++i) {
            elem_free(A->arr[i]);
        }
    }
    free(A->arr);
    free(A);
}



