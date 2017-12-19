#include <inttypes.h>
#include "colon.h"
#include "hdict.h"

#ifndef _HT_SYMB_H_
#define _HT_SYMB_H_

typedef struct ht_symb ht_symb_t;
struct ht_symb {
    hdict_t ht;
    ht_symb_t *next;
};

ht_symb_t *ht_symb_new ();

ht_symb_t *add_scope (ht_symb_t *old);

void add_symb (ht_symb_t *Ht_symb_p, char *symb, symb_h *node);

symb_h *symb_lookup (ht_symb_t *Ht_symb_p, char *symb);


#endif
