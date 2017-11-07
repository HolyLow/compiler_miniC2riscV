%{
#include "node.h"
#define YYSTYPE Nodes
#include "main.tab.h"
/*#ifdef YYSTYPE
 #undef YYSTYPE
#endif*/
extern Nodes yylval;
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
int       { yylval.str = strdup(yytext); /*printf("get int\n");*/ return WORD_INT; }
main      { yylval.str = strdup(yytext); /*printf("get main\n");*/ return MAIN; }
return    { yylval.str = strdup(yytext); /*printf("get return\n");*/ return RETURN; }
while     { yylval.str = strdup(yytext); return WHILE; }
if        { yylval.str = strdup(yytext); return IF; }
else      { yylval.str = strdup(yytext); return ELSE; }
{id}      { yylval.str = strdup(yytext); /*printf("get ID \"%s\"\n", yytext);*/ return ID; }
{num}     { yylval.str = strdup(yytext); /*printf("get NUM \"%s\"\n", yytext);*/ return NUM; }
&&        { yylval.str = strdup(yytext); return AND; }
"||"      { yylval.str = strdup(yytext); return OR; }
==        { yylval.str = strdup(yytext); return EQ; }
!=        { yylval.str = strdup(yytext); return NE; }
"<"       { yylval.str = strdup(yytext); return LT; }
>         { yylval.str = strdup(yytext); return GT; }
[\{\}\[\]\(\)\+\-\*\/\!\;\=]         { yylval.str = strdup(yytext); /*printf("get \"%s\"\n", yytext);*/ return yytext[0]; }

%%
int yywrap()
{
  return 1;
}
