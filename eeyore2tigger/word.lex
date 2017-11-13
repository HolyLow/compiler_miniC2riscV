%{
#include "node.h"
/*#define YYSTYPE Nodes*/
#include "main.tab.h"
/*extern Nodes yylval;*/
%}

delim   [\ \t\n]
ws      {delim}+
letter  [A-Za-z]
digit   [0-9]
num     ([1-9]+{digit}*)|0
var     [Tt]{num}
label   l{num}
func    f_{letter}+
logicop "=="|">"|"<"
/* attention! arithop doesn't include '-' */
arithop  "&&"|"||"|"+"|"*"|"/"|"%"

%%

ws        ;
{num}     { yylval.str = strdup(yytext); return INTEGER; }
{var}     { yylval.str = strdup(yytext); return VARIABLE; }
{label}   { yylval.str = strdup(yytext); return LABEL; }
{func}    { yylval.str = strdup(yytext); return FUNCTION; }
{logicop} { yylval.str = strdup(yytext); return LOGICOP; }
{arithop} { yylval.str = strdup(yytext); return ARITHOP; }
[\[\]\-\!\=\:]    { yylval.str = strdup(yytext); return yytext[0]; }

%%
int yywrap()
{
  return 1;
}
