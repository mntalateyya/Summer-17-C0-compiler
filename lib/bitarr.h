#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#ifndef _BITARR_H_
#define _BITARR_H_

struct barr {
    uint64_t *array;
    size_t len;
    size_t set;
};

typedef struct barr *barr_t;

barr_t barr_new (size_t size);

void barr_set (barr_t B, size_t index);

void barr_unset (barr_t B, size_t index);

bool barr_get (barr_t B, size_t index);

size_t barr_countset (barr_t B);

bool barr_equal (barr_t B1, barr_t B2);

size_t barr_union(barr_t dest, barr_t source);

void barr_free (barr_t B);

#endif
