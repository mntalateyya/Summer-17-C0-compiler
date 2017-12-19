#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <assert.h>
#include "nfa.h"
#include "../lib/stack.h"
#include "../lib/contracts.h"

#define SP sizeof(void*)

char *strip (FILE *f) {
    char *res = calloc(1,sizeof(char));
    int size = 1;
    int index = 0;
    size_t n = 80;
    char *line = malloc(sizeof(char)*n);
    ssize_t len = getline(&line, &n, f);
    while (len!=-1) {
        int i=0;
        while(line[i]!='\n') {
            if(index>=size-1) {
                res = realloc(res, sizeof(char)*size*2);
                size *= 2;
            }
            if (!isspace(line[i]))
                res[index++] = line[i];
            ++i;
        }
        len = getline(&line, &n, f);
    }

    res[index] = '\0';
    res = realloc(res, sizeof(char)*(index+1));
    free(line);
    return res;
}

bool is_nfa_p (nfa_p N) {
    if (!N) return false;
    if (!N->states || !N->start) return false;
    if (N->end_c>0 && !N->end) return false;
    int n, m;
    n= N->start->len;
    m = N->states->len;
    for (int i = 0; i<n; ++i) {
        bool included = false;        
        for (int j = 0; j<m; ++j) {
            if (N->start->arr[i]==N->states->arr[j]) included = true;
        }
        if (!included) return false;
    }
    n = N->end_c;
    for (int i=0; i<n; ++i) {
        bool included = false;
        m = N->states->len;
        for (int j=0; j<m; ++j) {
            if (N->states->arr[j]->letters <= N->end[i]
                && N->end[i] < (N->states->arr[j]->letters+128))
                included = true;
        }
        if (!included) return false;
    }
    return true;
}

void free_starr (nfa_starr St) {
    if (!St) return;
    free(St->arr);
    free(St);
}

//non-destructive
static void append (nfa_starr *dist, nfa_starr *source) {
    if (*dist==NULL) {
        *dist = (struct state_arr*)malloc(sizeof(struct state_arr));
        (*dist)->arr = NULL;
        (*dist)->len = 0;
    }
    (*dist)->arr = realloc((*dist)->arr, SP*((*dist)->len+(*source)->len));
    memcpy((*dist)->arr+(*dist)->len, (*source)->arr, SP*(*source)->len);
    (*dist)->len = (*dist)->len+(*source)->len;
}

//destructive
//merge A1 and A2 into a single states array
static nfa_starr merge (nfa_starr A1, nfa_starr A2) {
    if (A1==NULL) return A2;
    if (A2==NULL) return A1;
    A1->arr = realloc(A1->arr, SP*(A1->len+A2->len));
    memcpy(A1->arr+A1->len, A2->arr, SP*A2->len);
    A1->len = A1->len+A2->len;
    free_starr(A2);
    return A1;
}

//create an empty state of an nfa
static nfa_st create_state (int type) {
    static int id=0;
    nfa_st S = malloc(sizeof(struct state));
    S->letters = calloc(128,SP);    
    S->type = type;
    S->id = id;
    ++id;
    return S;
}

nfa_p char2nfa (uint8_t chr, int type) {
    nfa_p N = malloc(sizeof(struct nfa));
    N->states = malloc(sizeof(struct state_arr));
    N->states->arr = malloc(SP);
    N->states->len = 1;
    N->states->arr[0] = create_state(type);    
    
    N->start = malloc(sizeof(struct state_arr));
    N->start->arr = malloc(SP);
    N->start->len = 1;
    N->start->arr[0] = N->states->arr[0];
    
    N->end = malloc(SP);
    N->end[0] = N->states->arr[0]->letters+chr;
    N->end_c = 1;
    
    ENSURES(is_nfa_p(N));
    return N;
}


nfa_p concat (nfa_p N1, nfa_p N2) {
    REQUIRES(is_nfa_p(N1) && is_nfa_p(N2));
    N1->states = merge (N1->states, N2->states);
    nfa_starr **dest = N1->end; nfa_starr *source = &N2->start;
    for (int i=0; i<N1->end_c; ++i) {
        append(dest[i], source);
    }
    free(N1->end);
    N1->end = N2->end;
    N1->end_c = N2->end_c;
    free_starr(N2->start);
    free(N2);
    ENSURES(is_nfa_p(N1));
    return N1;
}
    
nfa_p star (nfa_p N) {
    REQUIRES(is_nfa_p(N));
    nfa_p tmp = char2nfa('\0', 0);
    tmp->states->arr[0]->letters['\0'] = N->start;
    nfa_starr A = malloc(sizeof(struct state_arr));
    A->arr = malloc(SP);
    A->len = 1;
    *A->arr = tmp->states->arr[0];
    
    nfa_starr **dest = N->end; nfa_starr *source = &A;
    for (int i=0; i<N->end_c; ++i) {
        append(dest[i], source);
    }
    free_starr(A);
    free(N->end);
    tmp->states = merge(N->states, tmp->states);
    free(N);
    ENSURES(is_nfa_p(tmp));
    return tmp;
}

nfa_p plus (nfa_p N) {
    REQUIRES(is_nfa_p(N));
    nfa_starr **dest = N->end; nfa_starr *source = &N->start;
    for (int i=0; i<N->end_c; ++i) {
        append(dest[i], source);
    }
    ENSURES(is_nfa_p(N));
    return N;
}

nfa_p union_ (nfa_p N1, nfa_p N2) {
    REQUIRES(is_nfa_p(N1) && is_nfa_p(N2));
    N1->states = merge(N1->states, N2->states);
    N1->start = merge(N1->start, N2->start);
    N1->end = realloc(N1->end, SP*(N1->end_c+N2->end_c));
    memcpy(N1->end+N1->end_c, N2->end, SP*N2->end_c);
    N1->end_c += N2->end_c;
    free(N2->end);
    free(N2);
    ENSURES(is_nfa_p(N1));
    return N1;
}

nfa_p escseq (char c) {
    nfa_p tmp;
    switch (c) {
        case 'n':
            return char2nfa('\n', 0);
        case '\\':
            return char2nfa('\\', 0);
        case '(':
            return char2nfa('(', 0);
        case ')':
            return char2nfa(')', 0);
        case '|':
            return char2nfa('|', 0);
        case '*':
            return char2nfa('*', 0);   
        case '+':
            return char2nfa('+', 0);
        case '?':
            return char2nfa('?', 0);
        case '@':
            return char2nfa('@', 0);
        case '[':
            return char2nfa('[', 0);
        case ']':
            return char2nfa(']', 0);
        case '^':
            return char2nfa('^', 0);
        case 's':
            return char2nfa(' ', 0);
        case 'w':
            tmp = char2nfa(' ', 0);
            tmp->end = realloc(tmp->end, SP*6);
            char *spaces = " \t\r\n\f\v";
            nfa_starr **dest = tmp->end; nfa_starr *source = tmp->states->arr[0]->letters;
            for (int i=0; i<6; ++i) {
                dest[i] = source+spaces[i];
            }
            tmp->end_c = 6;
            ENSURES(is_nfa_p(tmp));
            return tmp;
        default:
            fprintf(stderr, "INvalid regex (\\%c not recognized)\n", c);
            exit(1);
    }
}

nfa_p choices (char *str, int *i) {
    bool *chars = calloc(128,sizeof(bool));
    bool complement = (str[*i]=='^');
    if (complement) ++(*i);
    if (str[*i]==']') {
        fprintf(stderr, "@%d: Invalid regex (empty choices)\n", *i);
        exit(1);
    }
    
    while (str[*i] && str[*i]!=']') {
        unsigned char start = str[*i];
        if (str[*i+1] == '-')  {
            *i+=2;
            if (!str[*i]) {
                fprintf(stderr, "@%d: Invalid regex (incomplete range)\n", *i);
                exit(1);
            }
            unsigned char end = str[*i];
            for (; start<=end; ++start) {
                chars[start] = true;
            }
            ++(*i);
        } else {
            chars[start] = true;
            ++(*i);
        }
    }
    if (!str[*i]) {
        fprintf(stderr, 
        "@%d: Invalid regex (reached end before end of choices)\n", *i);
        exit(1);
    }
    if (complement) {
        for (unsigned char idx=0; idx<128; ++idx) {
            chars[idx] = !chars[idx];
        }
    }
    int count = 0;
    for (unsigned char idx=0; idx<128; ++idx) {
        if (chars[idx]) ++count;
    }
    nfa_p result = char2nfa('\0', 0);
    result->end = realloc(result->end, SP*count);
    
    nfa_starr **dest = result->end; nfa_starr *source = result->states->arr[0]->letters;
    int counter = 0;
    for (unsigned char idx=0; idx<128; ++idx) {
        if (chars[idx]) {
            dest[counter] = (source+idx);
            ++counter;
        }
    }
    result->end_c = count;
    free(chars);
    ENSURES(is_nfa_p(result));
    return result;
}

nfa_t parse_nfa (char *str) {
    stack_t nfas = stack_new(); //stack of nfas under processing
    nfa_t result = malloc(sizeof(struct nfa_parse));
    result->tags = malloc(SP);
    result->num_tags = 0;
    int i = 0; //index of char at str
    push(nfas, NULL); //starting separator
    nfa_p sep = malloc(sizeof(struct nfa)); //special separaor for | operator
    nfa_p obj, opsep, tmp; //temporary switch variables
    while (str[i]) {
        switch(str[i]) {
            case '(': 
#ifdef PRINTERROR
                fprintf(stderr, "@%d: open bracket\n", i);
#endif
                /*push new separator*/
                push(nfas, NULL);
                ++i;
                break;
                
            case ')':
#ifdef PRINTERROR
                fprintf(stderr, "@%d: close bracket\n", i);
#endif
                /* complete all | operations inside brackets then concat with
                previous nfa unless succeeded by *,+ or ? */
                if (!peek(nfas)) { //empty brackets
                    fprintf(stderr, "@%d: Invalid regex (NULL before ')')\n", i);
                    exit(1);
                }
                obj = pop(nfas); //nfa for an expression
                opsep = pop(nfas); //'|' separator
                while (opsep) { //loop until NULL (open bracket) found
                    if (opsep!=sep) {
                        fprintf(stderr, 
                        "@%d: Invalid regex (second before ')' not '|')\n", i);
                        exit(1);
                    }
                    tmp = pop(nfas);
                    if (!tmp) {
                        fprintf(stderr, "@%d: Invalid regex (NULL before '|')\n", i);
                        exit(1);
                    }
#ifdef PRINTERROR
                    fprintf(stderr, "@%d: close bracket: union_\n", i);
#endif
                    obj = union_(tmp,obj);
                    opsep = pop(nfas);
                }
                //if next is +, ? or * push bracket exp to stack and break
                if (str[i+1]=='*' || str[i+1]=='+' || str[i+1]=='?') {
#ifdef PRINTERROR
                    fprintf(stderr, "@%d: close bracket -> +?*\n", i);
#endif
                    push(nfas, obj);
                    ++i;
                    break;
                }
                //if prev is open bracket or | sep, push nfa to stack and break
                tmp = pop(nfas);
                if (!tmp || tmp==sep) {
#ifdef PRINTERROR
                    fprintf(stderr, "@%d: close bracket <- ( or |\n", i);
#endif
                    push(nfas, tmp);
                    push(nfas, obj);
                    ++i;
                    break;
                } else { //restore nfa to stack
                    push(nfas, tmp);
                }
#ifdef PRINTERROR
                fprintf(stderr, "@%d: close bracket: concat\n", i);
#endif
                obj = concat(pop(nfas), obj);
                push(nfas, obj);
                ++i;
                break;
                
            case '|':
                push(nfas, sep);
#ifdef PRINTERROR
                fprintf(stderr, "@%d: push sep\n", i);
#endif
                ++i;
                break;
                
            case '*':
                obj =  pop(nfas);
                if (!obj || obj == sep) { //invalid exp before *
                    fprintf(stderr, "@%d: Invalid regex (before '*' invalid)\n", i);
                    exit(1);
                }
                //* cannot be succeeded by *,? or +
                if (str[i+1]=='*' || str[i+1]=='?' || str[i+1]=='+') {
                    fprintf(stderr, 
                    "@%d: Invalid regex (redundant symbol after '*')\n", i);
                    exit(1);
                }
#ifdef PRINTERROR
                fprintf(stderr, "@%d: *\n", i);
#endif
                obj = star(obj);
                //if prev is open bracket or | sep, push nfa to stack and break
                tmp = pop(nfas);
                if (!tmp || tmp==sep) {
#ifdef PRINTERROR
                    fprintf(stderr, "@%d: * <- ( or |\n", i);
#endif
                    push(nfas, tmp);
                    push(nfas, obj);
                    ++i;
                    break;
                } else { //restore nfa to stack
                    push(nfas, tmp);
                }
#ifdef PRINTERROR
                fprintf(stderr, "@%d: *: concat\n", i);
#endif
                obj = concat(pop(nfas), obj);
                push(nfas, obj);
                ++i;
                break;
            case '+':
                obj =  pop(nfas);
                if (!obj || obj == sep) { //invalid exp before *
                    fprintf(stderr, "@%d: Invalid regex (before '*' invalid)\n", i);
                    exit(1);
                }
                //* cannot be succeeded by *,? or +
                if (str[i+1]=='*' || str[i+1]=='?' || str[i+1]=='+') {
                    fprintf(stderr, 
                    "@%d: Invalid regex (redundant symbol after '+')\n", i);
                    exit(1);
                }
#ifdef PRINTERROR
                fprintf(stderr, "@%d: +\n", i);
#endif
                obj = plus(obj);
                //if prev is open bracket or | sep, push nfa to stack and break
                tmp = pop(nfas);
                if (!tmp || tmp==sep) {
#ifdef PRINTERROR
                    fprintf(stderr, "@%d: + <- ( or |\n", i);
#endif
                    push(nfas, tmp);
                    push(nfas, obj);
                    ++i;
                    break;
                } else { //restore nfa to stack
                    push(nfas, tmp);
                }
#ifdef PRINTERROR
                fprintf(stderr, "@%d: +: concat\n", i);
#endif
                obj = concat(pop(nfas), obj);
                push(nfas, obj);
                ++i;
                break;
            case '?':
                obj =  pop(nfas);
                if (!obj || obj == sep) { //invalid exp before *
                    fprintf(stderr, "@%d: Invalid regex (before '*' invalid)\n", i);
                    exit(1);
                }
                //* cannot be succeeded by *,? or +
                if (str[i+1]=='*' || str[i+1]=='?' || str[i+1]=='+') {
                    fprintf(stderr, 
                    "@%d: Invalid regex (redundant symbol after '?')\n", i);
                    exit(1);
                }
#ifdef PRINTERROR
                fprintf(stderr, "@%d: ?\n", i);
#endif
                obj = union_(obj, char2nfa('\0',0));
                //if prev is open bracket or | sep, push nfa to stack and break
                tmp = pop(nfas);
                if (!tmp || tmp==sep) {
#ifdef PRINTERROR
                    fprintf(stderr, "@%d: ? <- ( or |\n", i);
#endif
                    push(nfas, tmp);
                    push(nfas, obj);
                    ++i;
                    break;
                } else { //restore nfa to stack
                    push(nfas, tmp);
                }
#ifdef PRINTERROR
                fprintf(stderr, "@%d: ?: concat\n", i);
#endif
                obj = concat(pop(nfas), obj);
                push(nfas, obj);
                ++i;
                break;
            case '\\':
                obj = escseq(str[++i]);
                //if next is +, ? or * push bracket exp to stack and break
                if (str[i+1]=='*' || str[i+1]=='+' || str[i+1]=='?') {
                    push(nfas, obj);
                    ++i;
                    break;
                }
                //if prev is open bracket or | sep, push nfa to stack and break
                tmp = pop(nfas);
                if (!tmp || tmp==sep) {
                    push(nfas, tmp);
                    push(nfas, obj);
                    ++i;
                    break;
                } else { //restore nfa to stack
                    push(nfas, tmp);
                }
                obj = concat(pop(nfas), obj);
                push(nfas, obj);
                ++i;
                break;
                
            case '@':
                if (str[++i]!='"') {
                    fprintf(stderr, "@%d: Invalid regex (after @ is not \")\n", i);
                    exit(1);
                }
                if (str[++i]=='"') {
                    fprintf(stderr, "@%d: Invalid regex (empty tag)\n", i);
                    exit(1);
                }
                int start = i;
                while (str[i] && str[i]!='"') ++i;
                if (!str[i]) {
                    fprintf(stderr, "@%d: Invalid regex (incomplete tag)\n", i);
                    exit(1);
                }
                int end = i;
                char *tag = malloc(end-start+1);
                strncpy(tag, str+start, end-start);
                tag[end-start] = '\0';
                result->tags = realloc(result->tags, SP*(++(result->num_tags)+1));
                result->tags[result->num_tags] = tag;
#ifdef PRINTERROR
                fprintf(stderr, "@%d: tag: \"%s\"\n", i, tag);
#endif
                obj = pop(nfas);
                if(!obj || obj==sep) {
                    fprintf(stderr, "@%d: Invalid exp before tag\n", i);
                    exit(1);
                }
                obj = concat(obj, char2nfa('\0', result->num_tags));
                push(nfas, obj);
                i=end+1;
                break;
            
            case '[':
                ++i;
#ifdef PRINTERROR
                fprintf(stderr, "@%d: [\n", i);
#endif
                obj = choices(str, &i);
                if (str[i+1]=='*' || str[i+1]=='+' || str[i+1]=='?') {
#ifdef PRINTERROR
                    fprintf(stderr, "@%d: [] -> ?*+ \n", i);
#endif
                    push(nfas, obj);
                    ++i;
                    break;
                }
                //if prev is open bracket or | sep, push nfa to stack and break
                tmp = pop(nfas);
                if (!tmp || tmp==sep) {
#ifdef PRINTERROR
                    fprintf(stderr, "@%d: [] <- ( or |\n", i);
#endif
                    push(nfas, tmp);
                    push(nfas, obj);
                    ++i;
                    break;
                } else { //restore nfa to stack
                    push(nfas, tmp);
                }
#ifdef PRINTERROR
                fprintf(stderr, "@%d: [] concat\n", i);
#endif
                obj = concat(pop(nfas), obj);
                push(nfas, obj); ++i;
                break;
                
            default:
#ifdef PRINTERROR
                fprintf(stderr, "@%d: char '%c'\n", i, str[i]);
#endif
                obj = char2nfa(str[i], 0);
                //if next is +, ? or * push bracket exp to stack and break
                if (str[i+1]=='*' || str[i+1]=='+' || str[i+1]=='?') {
#ifdef PRINTERROR
                fprintf(stderr, "@%d: char '%c' -> ?*+ \n", i, str[i]);
#endif
                    push(nfas, obj);
                    ++i;
                    break;
                }
                //if prev is open bracket or | sep, push nfa to stack and break
                tmp = pop(nfas);
                if (!tmp || tmp==sep) {
#ifdef PRINTERROR
                    fprintf(stderr, "@%d: char '%c' <- | or (\n", i, str[i]);
#endif
                    push(nfas, tmp);
                    push(nfas, obj);
                    ++i;
                    break;
                } else { //restore nfa to stack
                    push(nfas, tmp);
                }
#ifdef PRINTERROR
                fprintf(stderr, "@%d: char '%c' concat\n", i, str[i]);
#endif
                obj = concat(pop(nfas), obj);
                push(nfas, obj);
                ++i;
        }
        
    }
    /* complete remaining | operations*/
    if (!peek(nfas)) { //empty brackets
        fprintf(stderr, "Invalid regex (empty)\n");
        exit(1);
    }
    obj = pop(nfas); //nfa for an expression
    opsep = pop(nfas); //'|' separator
    while (opsep) { //loop until NULL (open bracket) found
        if (opsep!=sep) {
            fprintf(stderr, 
            "Invalid regex (second before ')' not '|')\n");
            exit(1);
        }
        tmp = pop(nfas);
        if (!tmp || tmp==sep) {
            fprintf(stderr, "Invalid regex (invalid exp for '|')\n");
            exit(1);
        }
        obj = union_(tmp,obj);
        opsep = pop(nfas);
    }
    assert(stack_empty(nfas));
    stack_free(nfas);
    
    ASSERT(is_nfa_p(obj));
    result->states = obj->states;
    result->start = obj->start;
    result->own_tags = true;
    free(obj->end);
    free(obj);
    free(sep);
    
    return result;
}


void free_nfa_t (nfa_t N) {
    int size = N->states->len;
    for (int i=0; i<size; ++i) {
#ifdef PRINTERROR
        fprintf(stderr, "freeing state: %d\n", N->states->arr[i]->id);
#endif
        nfa_starr *starr = N->states->arr[i]->letters;
        for (unsigned char c=0; c<128; ++c) {
            free_starr(starr[c]);
        }
        free(N->states->arr[i]->letters);
        free(N->states->arr[i]);
    }
    free_starr(N->states);
    free_starr(N->start);
    if (N->own_tags) {
        char **ptr = N->tags;
        for (int i=1; i<=N->num_tags; ++i) {
            free(ptr[i]);
        }
        free(N->tags);
        
    }
    free(N);
}


