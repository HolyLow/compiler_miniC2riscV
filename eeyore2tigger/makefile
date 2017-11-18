CC = g++
LEX = flex -v
YACC = bison -d

a.out: lex.yy.o main.tab.o
	$(CC) -o a.out lex.yy.o main.tab.o

lex.yy.o: lex.yy.c main.tab.h node.h

main.tab.o: main.tab.h node.h

lex.yy.c: word.lex
	$(LEX) word.lex

main.tab.h main.tab.c: main.y
	$(YACC) main.y
