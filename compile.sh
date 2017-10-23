#!/bin/bash

yacc="main"
lex="word"
yacc_y="${yacc}.y"
lex_l="${lex}.lex"
yacc_c="${yacc}.tab.c"
lex_c="lex.yy.c"

target="main"

bison -d $yacc_y
flex -v $lex_l

gcc $lex_c $yacc_c -o $target

