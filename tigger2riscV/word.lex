%{
#include "node.h"
#define YYSTYPE Nodes
#include "main.tab.h"
extern Nodes yylval;
%}

delim   [\ \t\n]
ws      {delim}+
comment (\#.*\n)
letter  [A-Za-z]
digit   [0-9]
num     ([1-9]+{digit}*)|0
var     v{num}
label   l{num}
reg_x   x0
reg_s   (s0)|(s1)|(s2)|(s3)|(s4)|(s5)|(s6)|(s7)|(s8)|(s9)|(s10)|(s11)
reg_a   (a0)|(a1)|(a2)|(a3)|(a4)|(a5)|(a6)|(a7)
reg_t   (t0)|(t1)|(t2)|(t3)|(t4)|(t5)|(t6)
reg     {reg_x}|{reg_s}|{reg_a}|{reg_t}
func    f_{letter}+[A-Za-z0-9_]*
logicop "=="|">"|"<"|">="|"<="|"!="
/* attention! arithop doesn't include '-' */
arithop  "&&"|"||"|"+"|"*"|"/"|"%"

%%

ws        ;
{comment} ;
{num}     { yylval.str = strdup(yytext); /*printf("get %s\n", yytext);*/ return INTEGER; }
{var}     { yylval.str = strdup(yytext); /*printf("get %s\n", yytext);*/ return VARIABLE; }
{label}   { yylval.str = strdup(yytext); /*printf("get %s\n", yytext);*/ return WORD_LABEL; }
{reg}     { yylval.str = strdup(yytext); /*printf("get %s\n", yytext);*/ return REG; }
{func}    { yylval.str = strdup(yytext); /*printf("get %s\n", yytext);*/ return FUNCTION; }
{logicop} { yylval.str = strdup(yytext); /*printf("get %s\n", yytext);*/ return LOGICOP; }
{arithop} { yylval.str = strdup(yytext); /*printf("get %s\n", yytext);*/ return ARITHOP; }
[\[\]\-\!\=\:]    { yylval.str = strdup(yytext); /*printf("get %s\n", yytext);*/ return yytext[0]; }
malloc    { /*printf("get %s\n", yytext);*/return WORD_MALLOC; }
end       { /*printf("get %s\n", yytext);*/return WORD_END; }
if        { /*printf("get %s\n", yytext);*/return WORD_IF; }
goto      { /*printf("get %s\n", yytext);*/return WORD_GOTO; }
call      { /*printf("get %s\n", yytext);*/return WORD_CALL; }
store     { /*printf("get %s\n", yytext);*/return WORD_STORE; }
load      { /*printf("get %s\n", yytext);*/return WORD_LOAD; }
loadaddr  { /*printf("get %s\n", yytext);*/return WORD_LOADADDR; }
return    { /*printf("get %s\n", yytext);*/return WORD_RETURN; }

%%
int yywrap()
{
  return 1;
}
