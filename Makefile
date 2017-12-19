CC = gcc
CFLAGS = -g -Wall -Wextra -Wshadow -Werror -std=c99 -pedantic

export_dfa: lexer/export_dfa.c  lexer/nfa2dfa.c lexer/nfa.c lib/*.c
	$(CC) $(CFLAGS) lib/*.c lexer/export_dfa.c lexer/nfa2dfa.c lexer/nfa.c -o export_dfa

test_parse: parser/test_parse.c lexer/read_dfa.c parser/parse_g.c lib/*.c
	$(CC) $(CFLAGS) -DDEBUG parser/test_parse.c  lexer/read_dfa.c parser/parse_g.c lib/*.c -o test_parse
 
TEST = test
test: unit_tests.c
	$(CC) $(CFLAGS) -o $(TEST) unit_tests.c lib/*.c

clean:
	$(RM) test_parse export_dfa
