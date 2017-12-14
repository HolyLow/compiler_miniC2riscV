%{
#include "node.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

/*Env env;*/

int yylex();
#define YYSTYPE Nodes
int yyerror(char *msg)
{
  printf("Error encountered: %s \n", msg);
}


string code;


%}

%token <str> VARIABLE INTEGER FUNCTION REG WORD_LABEL
%token WORD_MALLOC WORD_END WORD_IF WORD_GOTO WORD_CALL WORD_STORE WORD_LOAD WORD_LOADADDR WORD_RETURN
%token <str> LOGICOP ARITHOP '-' '!'

/*%type Goal GoalPart GlobalVarDecl FunctionDecl ExpressionList Expression Reg Label Function
%type Op1 Op2 LogicalOp*/
%type <str> Function

%%

Goal
: GoalPart {
    printf("tigger2riscV goal recognized!\n");
    printf("generated code is:\n%s", code.c_str());
  }
;
GoalPart
: GoalPart FunctionDecl
| GoalPart GlobalVarDecl
|
;
GlobalVarDecl
: VARIABLE '=' INTEGER {
    code += "\
            \t.global  global_var\n\
            \t.section .sdata\n\
            \t.align   2\n\
            \t.type    " + (string)$1 + ", @object\n\
            \t.size    " + (string)$1 + ", 4\n\
            " + (string)$1 + ":\n\
            \t.word    " + (string)$1 + "\n\n";
  }
| VARIABLE '=' WORD_MALLOC INTEGER {
    char addr[100];
    sprintf(addr, "%d", atoi($4) * 4);
    code += "\
            \t.comm     " + (string)$1 + ", " + (string)addr + ",4\n\n";
  }
;
FunctionDecl
: Function '[' INTEGER ']' '[' INTEGER ']' {
    char* func = &($1[2]);
    int nstk = (atoi($6) / 4 + 1) * 16;
    char stk[100];
    sprintf(stk, "%d", nstk);
    char offset_stk[100];
    sprintf(offset_stk, "%d", nstk - 4);
    code += "\
            \t.text\n\
            \t.align    2\n\
            \t.global   " + (string)func + "\n\
            \t.type     @" + (string)func + "\n\
            " + (string)func + ":\n\
            \tadd       sp, sp, -" + (string)stk + "\n\
            \tsw        ra, " + offset_stk + "(sp)\n";
  }
  ExpressionList WORD_END Function {
    char* func = &($11[2]);
    code += "\
            \t.size     " + (string)$3 + ", .-" + (string)func + "\n\n";
  }
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
: FUNCTION { $$ = $1; }
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
  code.clear();
  yyparse();
  printf("\n"); // to fix a really strange bug... to prevent a SEGMENTATION FAULT
  return 0;
}
