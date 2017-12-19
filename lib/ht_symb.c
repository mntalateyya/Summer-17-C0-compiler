#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include "ht_symb.h"
#include "colon.h"

bool equal_symb (hdict_key x, hdict_key y) {
    return strcmp((char*)x, (char*)y);
}

size_t hash_symb (hdict_key x) {
    char *str = (char*)x;
    size_t hash = 1327294621657427659ul;
    while (*str++) {
        hash = hash * 1664525 + 1013904223;
    }
    return hash;
}


ht_symb_t *ht_symb_new () {
    ht_symb_t *Ht_symb_p  = malloc(sizeof(ht_symb_t));
    Ht_symb_p->ht = hdict_new(32, equal_symb, hash_symb, NULL);
    Ht_symb_p->next = NULL;
    return Ht_symb_p;
}

ht_symb_t *add_scope (ht_symb_t *old) {
    ht_symb_t *new_ht_p = ht_symb_new();
    new_ht_p->next = old;
    return new_ht_p;
}

void add_symb (ht_symb_t *Ht_symb_p, char *symb, symb_h *node) {
    hdict_insert(Ht_symb_p->ht, symb, node);
}

symb_h *symb_lookup (ht_symb_t *Ht_symb_p, char *symb) {
    symb_h *node = NULL;
    while (!node && Ht_symb_p) {
        node = (symb_h*)hdict_lookup(Ht_symb_p->ht, symb);
        Ht_symb_p = Ht_symb_p->next;
    }
    return node;
}
