#include <stdbool.h>

#ifndef _COLON_H_
#define _COLON_H_
/*struct program {

};
typedef struct program prog_h;

struct function {

};
typedef struct function func_h;

struct block {

};
typedef struct block block_h;

struct expression {

};
typedef struct expression exp_h;*/

struct type {
    int type;
    struct type *next;
};
typedef struct type type_h;

struct symb_node {
    char *symb;
    type_h type;
    bool init;
};
typedef struct symb_node symb_h;


#endif //_COLON_H_
