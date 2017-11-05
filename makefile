CC = g++
LEX = flex -v
YACC = bison -d

main: lex.yy.o main.tab.o
	$(CC) -o main lex.yy.o main.tab.o

lex.yy.o: lex.yy.c main.tab.h util.h node.h

main.tab.o: main.tab.h util.h node.h

lex.yy.c: word.lex
	$(LEX) word.lex

main.tab.h main.tab.c: main.y
	$(YACC) main.y
