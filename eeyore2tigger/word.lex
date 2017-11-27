%{
#include "node.h"
#define YYSTYPE Nodes
#include "main.tab.h"
extern Nodes yylval;
%}

delim   [\ \t\n]
ws      {delim}+
letter  [A-Za-z]
digit   [0-9]
num     ([1-9]+{digit}*)|0
var     [Ttp]{num}
label   l{num}
func    f_{letter}+[A-Za-z0-9_]*
logicop "=="|">"|"<"
/* attention! arithop doesn't include '-' */
arithop  "&&"|"||"|"+"|"*"|"/"|"%"

%%

ws        ;
{num}     { yylval.str = strdup(yytext); printf("get %s\n", yytext); return INTEGER; }
{var}     { yylval.str = strdup(yytext); printf("get %s\n", yytext); return VARIABLE; }
{label}   { yylval.str = strdup(yytext); printf("get %s\n", yytext); return WORD_LABEL; }
{func}    { yylval.str = strdup(yytext); printf("get %s\n", yytext); return FUNCTION; }
{logicop} { yylval.str = strdup(yytext); printf("get %s\n", yytext); return LOGICOP; }
{arithop} { yylval.str = strdup(yytext); printf("get %s\n", yytext); return ARITHOP; }
[\[\]\-\!\=\:]    { yylval.str = strdup(yytext); printf("get %s\n", yytext); return yytext[0]; }
var       { return VAR; }
end       { return END; }
if        { return IF; }
goto      { return WORD_GOTO; }
param     { return WORD_PARAM; }
call      { return WORD_CALL; }
return    { return WORD_RETURN; }

%%
int yywrap()
{
  return 1;
}
