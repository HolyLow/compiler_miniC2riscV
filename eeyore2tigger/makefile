CC = g++
LEX = flex -v
YACC = bison -d
DEBUG = -g -rdynamic
a.out: lex.yy.o main.tab.o util.o
	$(CC) $(DEBUG) -o a.out lex.yy.o main.tab.o util.o 

lex.yy.o: lex.yy.c main.tab.h node.h

main.tab.o: main.tab.h node.h

util.o: util.c util.h

lex.yy.c: word.lex
	$(LEX) word.lex

main.tab.h main.tab.c: main.y
	$(YACC) main.y
