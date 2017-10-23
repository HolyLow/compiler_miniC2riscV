%{
#include "main.tab.h"
%}

delim   [\ \t\n]
ws      {delim}+
letter  [A-Za-z]
digit   [0-9]
comment (\/\*[^*]*([^*]*\*+[^*/])*[^*]*\*+\/)|(\/\/.*\n)
id      {letter}+[{letter}{digit}_]*
num     ([1-9]+{digit}*)|0

%%

ws        ;
{comment} ;
int       { yylval = strdup(yytext); printf("get int\n"); return INT; }
main      { yylval = strdup(yytext); printf("get main\n"); return MAIN; }
return    { yylval = strdup(yytext); printf("get return\n"); return RETURN; }
while     { yylval = strdup(yytext); return WHILE; }
if        { yylval = strdup(yytext); return IF; }
else      { yylval = strdup(yytext); return ELSE; }
{id}      { yylval = strdup(yytext); printf("get ID \"%s\"\n", yytext); return ID; }
{num}     { yylval = strdup(yytext); return NUM; }
&&        { yylval = strdup(yytext); return AND; }
"||"      { yylval = strdup(yytext); return OR; }
==        { yylval = strdup(yytext); return EQ; }
!=        { yylval = strdup(yytext); return NE; }
"<"       { yylval = strdup(yytext); return LT; }
>         { yylval = strdup(yytext); return GT; }
[\{\}\[\]\(\)\+\-\*\/\!\;\=]         { yylval = strdup(yytext); printf("get \"%s\"\n", yytext); return yytext[0]; }

%%
int yywrap()
{
  return 1;
}
