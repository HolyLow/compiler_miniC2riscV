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

void word_error(string msg, string name) {
  printf("%s: %s\n", msg.c_str(), name.c_str());
  exit(1);
}

string code;
int stack;

%}

%token <str> VARIABLE INTEGER FUNCTION REG WORD_LABEL
%token WORD_MALLOC WORD_END WORD_IF WORD_GOTO WORD_CALL WORD_STORE WORD_LOAD WORD_LOADADDR WORD_RETURN
%token <str> LOGICOP ARITHOP '-' '!'

/*%type Goal GoalPart GlobalVarDecl FunctionDecl ExpressionList Expression Reg Label Function
%type Op1 Op2 LogicalOp*/
%type <_string> Function Reg Label
%type <op>  Op1 Op2 LogicalOp

%%

Goal
: GoalPart {
    // printf("tigger2riscV goal recognized!\n");
    //printf("generated code is:\n%s", code.c_str());
    printf("\n%s", code.c_str());
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
            \t.word    " + (string)$3 + "\n\n";
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
    int nstk = (atoi($6) / 4 + 1) * 16;
    stack = nstk;
    char stk[100];
    sprintf(stk, "%d", nstk);
    char offset_stk[100];
    sprintf(offset_stk, "%d", nstk - 4);
    code += "\
            \t.text\n\
            \t.align    2\n\
            \t.global   " + $1 + "\n\
            \t.type     @" + $1 + "\n\
            " + $1 + ":\n\
            \tadd       sp, sp, -" + (string)stk + "\n\
            \tsw        ra, " + offset_stk + "(sp)\n";
  }
  ExpressionList WORD_END Function {
    code += "\
            \t.size     " + $1 + ", .-" + $1 + "\n\n";
  }
;
ExpressionList
: ExpressionList Expression
|
;
Expression
: Reg '=' Reg Op2 Reg {
    switch($4) {
      case ADD:
        code +=
            "\
            \tadd       " + $1 + ", " + $3 + ", " + $5 +"\n";
        break;
      case SUBTRACT:
        code +=
            "\
            \tsub       " + $1 + ", " + $3 + ", " + $5 +"\n";
        break;
      case MULTIPLY:
        code +=
            "\
            \tmul       " + $1 + ", " + $3 + ", " + $5 +"\n";
        break;
      case DIVIDE:
        code +=
            "\
            \tdiv       " + $1 + ", " + $3 + ", " + $5 +"\n";
        break;
      case MOD:
        code +=
            "\
            \trem       " + $1 + ", " + $3 + ", " + $5 +"\n";
        break;
      case LT:
        code +=
            "\
            \tslt       " + $1 + ", " + $3 + ", " + $5 +"\n";
        break;
      case GT:
        code +=
            "\
            \tsgt       " + $1 + ", " + $3 + ", " + $5 +"\n";
        break;
      case AND:
        code +=
            "\
            \tand       " + $1 + ", " + $3 + ", " + $5 +"\n\
            \tsnez      " + $1 + ", " + $1 + "\n";
        break;
      case OR:
        code +=
            "\
            \tor        " + $1 + ", " + $3 + ", " + $5 +"\n\
            \tsnez      " + $1 + ", " + $1 + "\n";
        break;
      /* case NE: */
      case EQ:
        code +=
            "\
            \txor       " + $1 + ", " + $3 + ", " + $5 +"\n\
            \tseqz      " + $1 + ", " + $1 + "\n";
        break;
      default:
        word_error("unsupported op for Reg = Reg Op2 Reg", "...");
    }
  }
| Reg '=' Reg Op2 INTEGER {
    switch($4) {
      case ADD:
        code +=
            "\
            \tadd       " + $1 + ", " + $3 + ", " + $5 +"\n";
        break;
      case LT:
        code +=
            "\
            \tslti      " + $1 + ", " + $3 + ", " + $5 +"\n";
        break;
      default:
        word_error("unsupported op for Reg = Reg Op2 INTEGER", "...");
    }
  }
| Reg '=' Op1 Reg {
    switch($3) {
      case NOT:
        code +=
            "\
            \tseqz      " + $1 + ", " + $4 + "\n"; // not sure whether to use "seqz" or "not"
        break;
      case NEG:
        code +=
            "\
            \tneg       " + $1 + ", " + $4 + "\n";
        break;
      default:
        word_error("unsupported op for Reg = Op1 Reg", "...");
    }
  }
| Reg '=' Reg {
    code += "\
            \tmv        " + $1 + ", " + $3 + "\n";
  }
| Reg '=' INTEGER {
    code += "\
            \tli        " + $1 + ", " + $3 + "\n";
  }
| Reg '[' INTEGER ']' '=' Reg {
    code += "\
            \tsw        " + $6 + ", " + (string)$3 + "(" + $1 + ")\n";
  }
| Reg '=' Reg '[' INTEGER ']' {
    code += "\
            \tlw        " + $1 + ", " + (string)$5 + "(" + $3 + ")\n";
  }
| WORD_IF Reg LogicalOp Reg WORD_GOTO Label {
    switch($3) {
      case LT:
        code += "\
            \tblt       " + $2 + ", " + $4 + ", " + $6 + "\n";
        break;
      case GT:
        code += "\
            \tbgt       " + $2 + ", " + $4 + ", " + $6 + "\n";
        break;
      case NE:
        code += "\
            \tbne       " + $2 + ", " + $4 + ", " + $6 + "\n";
        break;
      case EQ:
        code += "\
            \tbeq       " + $2 + ", " + $4 + ", " + $6 + "\n";
        break;
      case LE:
        code += "\
            \tble       " + $2 + ", " + $4 + ", " + $6 + "\n";
        break;
      case GE:
        code += "\
            \tbge       " + $2 + ", " + $4 + ", " + $6 + "\n";
        break;
      default:
        word_error("unsupported op for IF GOTO", "...");
    }
  }
| WORD_GOTO Label {
    code += "\
            \tj         " + $2 + "\n";
  }
| Label ':' {
    code += "\
            ." + $1 + ":\n";
  }
| WORD_CALL Function {
    code += "\
            \tcall      " + $2 + "\n";
  }
| WORD_STORE Reg INTEGER {
    char offset[100];
    int n_offset = atoi($3) * 4;
    sprintf(offset, "%d", n_offset);
    code += "\
            \tsw        " + $2 + ", " + (string)offset + "(sp)\n";
  }
| WORD_LOAD INTEGER Reg{
    char offset[100];
    int n_offset = atoi($2) * 4;
    sprintf(offset, "%d", n_offset);
    code += "\
            \tlw        " + $3 + ", " + (string)offset + "(sp)\n";
  }
| WORD_LOAD VARIABLE Reg {
    code += "\
            \tlui       " + $3 + ", %hi(" + (string)$2 + ")\n\
            \tlw        " + $3 + ", %lo(" + $2 + ")(" + $3 +")\n";
  }
| WORD_LOADADDR INTEGER Reg {
    char offset[100];
    int n_offset = atoi($2) * 4;
    sprintf(offset, "%d", n_offset);
    code += "\
            \tadd       " + $3 + ", sp, " + (string)offset + "\n";
  }
| WORD_LOADADDR VARIABLE Reg {
    code += "\
            \tlui       " + $3 + ", %hi(" + (string)$2 + ")\n\
            \tadd       " + $3 + ", " + $3 + ", %lo(" + (string)$2 + ")\n";
  }
| WORD_RETURN {
    char stk[100];
    sprintf(stk, "%d", stack);
    char offset_stk[100];
    sprintf(offset_stk, "%d", stack - 4);
    code += "\
            \tlw        ra, " + (string)offset_stk + "(sp)\n\
            \tadd       sp, sp, " + (string)stk + "\n\
            \tjr        ra\n";
  }
;
Reg
: REG { $$ = $1; }
;
Label
: WORD_LABEL { $$ = $1; }
;
Function
: FUNCTION { $$ = &($1[2]); }
;
Op2
: LOGICOP {
    switch($1[0]) {
      case '=':
        $$ = EQ;
        break;
      case '>':
        $$ = GT;
        break;
      case '<':
        $$ = LT;
        break;
      default:
        word_error("undefined LOGICOP", $1);
    }
  }
| ARITHOP {
    switch($1[0]) {
      case '&':
        $$ = AND;
        break;
      case '|':
        $$ = OR;
        break;
      case '+':
        $$ = ADD;
        break;
      case '*':
        $$ = MULTIPLY;
        break;
      case '/':
        $$ = DIVIDE;
        break;
      case '%':
        $$ = MOD;
        break;
      default:
        word_error("undefined ARITHOP", $1);
    }
  }
| '-' { $$ = SUBTRACT; }
;
Op1
: '!' { $$ = NOT; }
| '-' { $$ = NEG; }
;
LogicalOp
: LOGICOP {
    switch($1[0]) {
      case '=':
        $$ = EQ;
        break;
      case '>':
        $$ = GT;
        break;
      case '<':
        $$ = LT;
        break;
      default:
        word_error("undefined LOGICOP", $1);
    }
  }
;

%%

int main()
{
  code.clear();
  yyparse();
  printf("\n"); // to fix a really strange bug... to prevent a SEGMENTATION FAULT
  return 0;
}
