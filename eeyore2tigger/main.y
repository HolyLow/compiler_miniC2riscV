%{
#include "node.h"
#include <stdio.h>


int yylex();
#define YYSTYPE Nodes
int yyerror(char *msg)
{
  printf("Error encountered: %s \n", msg);
}

%}


%token <str> INTEGER VARIABLE WORD_LABEL FUNCTION
%token VAR END IF WORD_GOTO WORD_PARAM WORD_CALL WORD_RETURN
%token <str> LOGICOP ARITHOP '-' '!'

%type <part> Function Label Variable LogicalOp Op1 Op2 RightValue
%%
Goal
: GoalPart { printf("goal recognized!\n"); }
;
GoalPart
: GoalPart VarDecl
| GoalPart FuncDecl
|
;
VarDecl
: VAR VarLength Variable
;
VarLength
: INTEGER
|
;
FuncDecl
: Function '[' INTEGER ']' FuncBody END Function
;
FuncBody
: FuncBody Expression
| FuncBody VarDecl
|
;
RightValue
: Variable
| INTEGER
;
Expression
: Variable '=' RightValue Op2 RightValue
| Variable '=' Op1 RightValue
| Variable '=' RightValue
| Variable '[' RightValue ']' '=' RightValue
| Variable '=' Variable '[' RightValue ']'
| IF RightValue LogicalOp RightValue WORD_GOTO Label
| WORD_GOTO Label
| Label ':'
| WORD_PARAM RightValue
| Variable '=' WORD_CALL Function
| WORD_CALL Function
| WORD_RETURN RightValue
;
Op2
: LOGICOP { $$ = $1; }
| ARITHOP { $$ = $1; }
| '-' { $$ = $1; }
;
Op1
: '!' { $$ = $1; }
| '-' { $$ = $1; }
;
LogicalOp
: LOGICOP { $$ = $1; }
;
Variable
: VARIABLE { $$ = $1; }
;
Label
: WORD_LABEL { $$ = $1; }
;
Function
: FUNCTION { $$ = $1; }
;

%%

int main()
{
  yyparse();
  return 0;
}
