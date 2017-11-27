%{
#include "node.h"
#include <stdio.h>

Env env;

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
%type <sent> Expression
%type <var>  VarDecl
%type <num>  VarLength
/*%type <sentlist> FuncBody*/
%type <func> FuncDecl FuncBody
%%
Goal
: GoalPart { printf("goal recognized!\n"); env.analyze(); printf("analyze finished!\n"); }
;
GoalPart
: GoalPart VarDecl {
    $2.isGlobal = true;
    env.addVar($2);
  }
| GoalPart FuncDecl {
    env.addFunc($2);
  }
| {}
;
VarDecl
: VAR VarLength Variable {
    if($2 != -1) {
      $$.isArray = true;
      $$.arrayLength = $2;
    }
    else {
      $$.isArray = false;
    }
    $$.name = $3;
  }
;
VarLength
: INTEGER { $$ = atoi($1); }
| { $$ = -1; }
;
FuncDecl
: Function '[' INTEGER ']' FuncBody END Function {
    $$ = $5;
    $$.set_name($1);
    $$.set_param_num(atoi($3));
  }
;
FuncBody
: FuncBody Expression {
    $$ = $1;
    $$.addSentence($2);
  }
| FuncBody VarDecl {
    $$ = $1;
    $2.isGlobal = false;
    $$.addVar($2);
    /*$2.isGlobal = false;
    $1.addVar($2);
    $$ = $1;*/
  }
| { $$.clear(); }
;
RightValue
: Variable { $$ = $1; }
| INTEGER { $$ = $1; }
;
Expression
: Variable '=' RightValue Op2 RightValue {
    $$.type = OP_2;
    $$.var1 = $1;
    $$.var2 = $3;
    $$.op = $4;
    $$.var3 = $5;
  }
| Variable '=' Op1 RightValue {
    $$.type = OP_1;
    $$.var1 = $1;
    $$.op = $3;
    $$.var2 = $4;
  }
| Variable '=' RightValue {
    $$.type = ASSIGN;
    $$.var1 = $1;
    $$.var2 = $3;
  }
| Variable '[' RightValue ']' '=' RightValue {
    $$.type = STORE;
    $$.var1 = $1;
    $$.var2 = $3;
    $$.var3 = $6;
  }
| Variable '=' Variable '[' RightValue ']' {
    $$.type = LOAD;
    $$.var1 = $1;
    $$.var2 = $3;
    $$.var3 = $5;
  }
| IF RightValue LogicalOp RightValue WORD_GOTO Label {
    $$.type = CON_JUMP;
    $$.var1 = $2;
    $$.op = $3;
    $$.var2 = $4;
    $$.var3 = $6;
  }
| WORD_GOTO Label {
    $$.type = JUMP;
    $$.var1 = $2;
  }
| Label ':' {
    $$.type = LABEL;
    $$.var1 = $1;
  }
| WORD_PARAM RightValue {
    $$.type = PARAM;
    $$.var1 = $2;
  }
| Variable '=' WORD_CALL Function {
    $$.type = CALL_ASSIGN;
    $$.var1 = $1;
    $$.var2 = $4;
  }
| WORD_CALL Function {
    $$.type = CALL;
    $$.var1 = $2;
  }
| WORD_RETURN RightValue {
    $$.type = RETURN;
    $$.var1 = $2;
  }
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
  printf("\n"); // to fix a really strange bug... to prevent a SEGMENTATION FAULT
  return 0;
}
