#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer/nfa.h"
#include "lib/colon.h"
#include "lib/ht_symb.h"

int main() {
    ht_symb_t *symb_table = ht_symb_new();
    (void)symb_table;
    FILE *f = fopen("test_regex.txt","r");
    if (!f) {
        puts("error reading file");
        return 1;
    }
    char *str = strip(f);
    printf("%s--end--\n", str);
    printf("length:%lu\n", strlen(str));
    return 0;
}
