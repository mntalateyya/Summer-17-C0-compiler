#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef TEST_BARR
#include "bitarr.h"
#endif

#ifdef TEST_UB_ARR 
#include "ubarray.h"
#endif

int main () {
//test bitarr
#ifdef TEST_BARR
    barr_t B = barr_new(10);
    assert(barr_countset(B)==0);
    for (size_t i=0; i<10; ++i) {
        barr_set(B, i);
        assert(barr_get(B,i));
    }
    assert(barr_countset(B)==10);
    for (size_t i=0; i<10; ++i) {
        barr_unset(B, i);
        assert(!barr_get(B,i));
    }
    assert(barr_countset(B)==0);
    barr_free(B);
    B = barr_new(100);
    assert(barr_countset(B)==0);
    for (size_t i=0; i<100; ++i) {
        barr_set(B, i);
        assert(barr_get(B,i));
    }
    assert(barr_countset(B)==100);
    for (size_t i=0; i<100; ++i) {
        barr_unset(B, i);
        assert(!barr_get(B,i));
    }
    assert(barr_countset(B)==0);
    barr_free(B);
#endif
    
#ifdef TEST_UB_ARR 
    char *x = "This is some text for testing";
    int y =1;
    ubarray_t A = array_new(10);
    for (size_t i=0; i<strlen(x); ++i) {
        assert(array_len(A)==i);
        array_add(A, x+i);
        assert(array_get(A, i)==x+i);
    }
    printf("max:%lu, len: %lu\n", A->max, A->len);
    for (size_t i=0; i<strlen(x); ++i) {
        printf("retrieve i: %lu, c: %c\n", i, x[i]);
        printf("==got: %d, is: %d\n", *(char*)array_get(A, i), x[i]);
        assert(array_get(A, i)==x+i);
    }
    array_set(A, 10, &y);
    assert(array_get(A, 10)==&y);
    array_free(A);
#endif

    

    return 0;
}
