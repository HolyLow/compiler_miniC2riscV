%{
#include "node.h"
#include <stdio.h>

/*Env env;*/

int yylex();
#define YYSTYPE Nodes
int yyerror(char *msg)
{
  printf("Error encountered: %s \n", msg);
}

%}

%token <str> VARIABLE INTEGER FUNCTION REG WORD_LABEL
%token WORD_MALLOC WORD_END WORD_IF WORD_GOTO WORD_CALL WORD_STORE WORD_LOAD WORD_LOADADDR WORD_RETURN
%token <str> LOGICOP ARITHOP '-' '!'

/*%type Goal GoalPart GlobalVarDecl FunctionDecl ExpressionList Expression Reg Label Function
%type Op1 Op2 LogicalOp*/
%%

Goal
: GoalPart { printf("tigger2riscV goal recognized!\n"); }
;
GoalPart
: GoalPart FunctionDecl
| GoalPart GlobalVarDecl
|
;
GlobalVarDecl
: VARIABLE '=' INTEGER
| VARIABLE '=' WORD_MALLOC INTEGER
;
FunctionDecl
: Function '[' INTEGER ']' '[' INTEGER ']' ExpressionList WORD_END Function
;
ExpressionList
: ExpressionList Expression
|
;
Expression
: Reg '=' Reg Op2 Reg
| Reg '=' Reg Op2 INTEGER
| Reg '=' Op1 Reg
| Reg '=' Reg
| Reg '=' INTEGER
| Reg '[' INTEGER ']' '=' Reg
| Reg '=' Reg '[' INTEGER ']'
| WORD_IF Reg LogicalOp Reg WORD_GOTO Label
| WORD_GOTO Label
| Label ':'
| WORD_CALL Function
| WORD_STORE Reg INTEGER
| WORD_LOAD INTEGER Reg
| WORD_LOAD VARIABLE Reg
| WORD_LOADADDR INTEGER Reg
| WORD_LOADADDR VARIABLE Reg
| WORD_RETURN
;
Reg
: REG
;
Label
: WORD_LABEL
;
Function
: FUNCTION
;
Op2
: LOGICOP
| ARITHOP
| '-'
;
Op1
: '!'
| '-'
;
LogicalOp
: LOGICOP
;

%%

int main()
{
  yyparse();
  printf("\n"); // to fix a really strange bug... to prevent a SEGMENTATION FAULT
  return 0;
}
