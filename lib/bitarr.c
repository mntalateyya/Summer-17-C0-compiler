#include <stdio.h>
#include "bitarr.h"
#include "contracts.h"

bool is_barr (barr_t B) {
    if (!B || !B->array) {
        printf("!b || !B->array\n");
        return false;
    }
    size_t count = 0;
    for (size_t i=0; i<(B->len+63)/64; ++i) {
        size_t tmp = B->array[i];
        while (tmp) {
            tmp &= tmp-1;
            ++count;
        }
    }
    return count == B->set;
    
}

barr_t barr_new (size_t size) {
    REQUIRES(size>0);
    barr_t B = malloc(sizeof(struct barr));
    B->array = calloc((size+63)/64, sizeof(uint64_t));
    B->len = size;
    B->set = 0;
    ENSURES(is_barr(B));
    return B;
}

void barr_set (barr_t B, size_t index) {
    REQUIRES(is_barr(B));
    REQUIRES(B->len>index);
    B->array[index/64] |= (size_t)1<<(index%64);
    (B->set)++;
    ENSURES(is_barr(B));
}

void barr_unset (barr_t B, size_t index) {
    REQUIRES(is_barr(B));
    REQUIRES(B->len>index);
    B->array[index/64] &= ~((size_t)1<<(index%64));
    (B->set)--;
    ENSURES(is_barr(B));
}

bool barr_get (barr_t B, size_t index) {
    REQUIRES(is_barr(B));
    REQUIRES(B->len>index);
    return (B->array[index/64]>>(index%64)) & 1;
}

size_t barr_countset (barr_t B) {
    REQUIRES(is_barr(B));
    return B->set;
}

bool barr_equal (barr_t B1, barr_t B2) {
    REQUIRES(is_barr(B1) && is_barr(B2));
    if (B1->len!=B2->len) return false;
    for (size_t i=0; i<(B1->len+63)/64; ++i) {
        if (B1->array[i]!=B2->array[i]) return false;
    }
    return true;
}

size_t barr_union(barr_t dest, barr_t source) {
    REQUIRES(is_barr(dest) && is_barr(source));
    ASSERT(dest->len == source->len);
    for (size_t i=0; i<(dest->len+63)/64; ++i) {
        dest->array[i] |= source->array[i];
    }
    size_t count = 0;
    for (size_t i=0; i<(dest->len+63)/64; ++i) {
        size_t tmp = dest->array[i];
        while (tmp) {
            tmp &= tmp-1;
            ++count;
        }
    }
    int prev = dest->set;
    dest->set = count;
    ENSURES(is_barr(dest));
    ENSURES(count -prev <= source->len);
    return count - prev;
}

void barr_free (barr_t B) {
    REQUIRES(is_barr(B));
    free(B->array);
    free(B);
}
