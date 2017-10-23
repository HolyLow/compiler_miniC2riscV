%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
%}

%token IF ELSE WHILE RETURN MAIN
%token INT
%token ID
%token NUM
%token AND OR EQ NE LT GT
%left  ','
%right '='
%left  OR
%left  AND
%left  EQ NE
%left  LT GT
%left  '+' '-'
%left  '*' '/' '%'
%right UMINUS '!'
%%
Goal
: GlobalD MainFunc {
  printf("Goal recognized!\n");
}
;
GlobalD
: GlobalD VarDefn
| GlobalD FuncDefn
| GlobalD FuncDecl
|
;
MainFunc
: INT {
    printf("before main\n");
}
  MAIN {
    printf("after main\n");
  }
  '(' ')' '{' StatementPack '}'
;
VarDefn
: Type Identifier ';'
| Type Identifier '[' NUM ']' ';'
;
VarDecl
: Type Identifier
| Type Identifier '[' NUM ']'
;
VarDeclPack
: VarDeclPack VarDecl
|
;
FuncDefn
: Type Identifier '(' VarDeclPack ')' '{' StatementPack '}'
;
FuncDecl
: Type Identifier '(' VarDeclPack ')' ';'
;
Type
: INT
;
Statement
: '{' StatementPack '}'
| IF '(' Expression ')' Statement
| IF '(' Expression ')' Statement ELSE Statement
| WHILE '(' Expression ')' Statement
| Identifier '=' Expression ';'
| Identifier '[' Expression ']' '=' Expression ';'
| Identifier '(' ExpressionList ')' ';'
| VarDefn
| RETURN Expression ';'
;
StatementPack
: StatementPack Statement
|
;
Expression
: NUM
| Identifier
| '-' Expression %prec UMINUS
| '!' Expression
| Identifier '(' ExpressionList ')'
| Expression '[' Expression ']'
| Expression '+' Expression
| Expression '-' Expression
| Expression '*' Expression
| Expression '/' Expression
| Expression AND Expression
| Expression OR Expression
| Expression EQ Expression
| Expression NE Expression
| Expression LT Expression
| Expression GT Expression
;
ExpressionList
: Expression ExpressionAppend
|
;
ExpressionAppend
: ',' Expression ExpressionAppend
|
;
Identifier
: ID
;
%%
int main()
{
  yyparse();
  return 0;
}

int yyerror(char *msg)
{
  printf("Error encountered: %s \n", msg);
}
