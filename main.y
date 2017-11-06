%{
#include <map>
#include <string>
#include <list>
#include "util.h"
#include "node.h"
using namespace std;

int yylex();

#define YYSTYPE Nodes

Env *root_env, *last_env, *this_env;

int ori_var_cnt, tmp_var_cnt;

void StartNewEnv()
{
  last_env = this_env;
  this_env = new Env(last_env);
}

void RollBackEnv()
{
  last_env = this_env->parent();
  this_env = last_env;
}

void AddVarToken(string id, TokenType type, string ename)
{
  this_env->insertVarToken(id, type, ename);
}

void AddFuncToken(string id, VarList var_list, TokenType return_type, bool isDef)
{
  ParamList plist; plist.clear();
  VarList::iterator it;
  for(it = var_list.begin(); it != var_list.end(); it++) {
    plist.push_back(it->type);
  }
  this_env->insertFuncToken(id, plist, return_type, isDef);
}

string SearchToken(string id, bool isVar)
{
  return this_env->searchToken(id, isVar);
}

void TransUnaryOp(Expr & result, Expr & right, char *op)
{
  char exp[100];
  char tmp[10];
  sprintf(tmp, "t%d", tmp_var_cnt++);
  sprintf(exp, "var %s\n%s = %s %s\n", tmp, tmp, op, right.result.c_str());
  result.code = right.code + string(exp);
  result.result = string(tmp);
}

void TransBinaryOp(Expr & result, Expr & left, Expr & right, char *op)
{
  char exp[100];
  char tmp[10];
  sprintf(tmp, "t%d", tmp_var_cnt++);
  sprintf(exp, "var %s\n%s = %s %s %s\n", tmp, tmp, left.result.c_str(), op, right.result.c_str());
  result.code = left.code + right.code + string(exp);
  result.result = string(tmp);
}

int yyerror(char *msg)
{
  printf("Error encountered: %s \n", msg);
}
%}

/*%union {
  struct Expr expr;
  struct Func func;
  struct Stat stat;
  string      iden;
  struct Part part;
  TokenType   type;
  ParamList   param_list;
  char * str;
  int    num;
}*/

%type <expr> Expression
%type <func> FuncDefn MainFunc
%type <stat> Statement
%type <iden> Identifier
%type <type> Type ParamDecl
%type <param_list> ParamDeclPack
%type <var_info> ParamDefn
%type <var_list> ParamDefnPack
%type <expr_list> ExpressionList ExpressionAppend
%type <part> Goal GlobalD VarDefn StatementPack


%token IF ELSE WHILE RETURN MAIN
%token WORD_INT
%token <str>  ID
/*%token <num> NUM*/
%token <str> NUM
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
: GlobalD VarDefn  { $$.code = $1.code + $2.code; }
| GlobalD FuncDefn { $$.code = $1.code + $2.code; }
| GlobalD FuncDecl { $$.code = $1.code; }
| { $$.code.clear(); }
;
MainFunc
: WORD_INT { printf("before main\n"); }
  MAIN { printf("after main\n"); }
  '(' ')' '{' { StartNewEnv(); }
  StatementPack '}' { RollBackEnv(); }
;
VarDefn
: Type Identifier ';' {
    char eeyore_name[10];
    sprintf(eeyore_name, "T%d", ori_var_cnt++);
    AddVarToken($2, $1, string(eeyore_name));
    char code[100];
    sprintf(code, "var %s\n", eeyore_name);
    $$.code = code;
  }
| Type Identifier '[' NUM ']' ';' {
    char eeyore_name[10];
    sprintf(eeyore_name, "T%d", ori_var_cnt++);
    AddVarToken($2, Array($1), string(eeyore_name));
    char code[100];
    sprintf(code, "var %d %s\n", 4*atoi($4), eeyore_name);
    $$.code = code;
  }
;
/*ParamDecl
: Type Identifier { $$ = $1; }
| Type Identifier '[' NUM ']' { $$ = Array($1); }
;
ParamDeclPack
: ParamDeclPack ParamDecl { $$ = $1; $$.insert($$.end(), $2); }
| { $$.clear(); }
;*/
ParamDefn
: Type Identifier { $$.type = $1; $$.minic_name = $2; }
| Type Identifier '[' NUM ']' { $$.type = Array($1); $$.minic_name = $2; }
;
ParamDefnPack
: ParamDefnPack ParamDefn { $$ = $1; $$.insert($$.end(), $2); }
| { $$.clear(); }
;
FuncDefn
: Type Identifier '(' ParamDefnPack ')'  { AddFuncToken($2, $4, $1, true); }
  '{' {
    StartNewEnv();
    VarList::iterator it;
    int param_cnt = 0;
    for(it = $4.begin(); it != $4.end(); it++) {
      char ename[10];
      sprintf(ename, "p%d", param_cnt++);
      AddVarToken(it->minic_name, it->type, string(ename));
    }
  }
  StatementPack '}' { RollBackEnv(); }
;
FuncDecl
: Type Identifier '(' ParamDefnPack ')' ';' { AddFuncToken($2, $4, $1, false); }
;
Type
: WORD_INT { $$ = INT; }
;
Statement
: '{' { StartNewEnv(); }
  StatementPack '}' { RollBackEnv(); }
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
: NUM { $$.result = string($1); $$.code.clear(); }
| Identifier { $$.result = SearchToken($1, true); $$.code.clear(); }
| '-' Expression %prec UMINUS { TransUnaryOp($$, $2, "-"); }
| '!' Expression              { TransUnaryOp($$, $2, "!"); }
| Identifier '(' ExpressionList ')' {
    string iden = SearchToken($1, false);
    char exp[500]; exp[0] = '\0';
    char tmp[10];
    sprintf(tmp, "t%d", tmp_var_cnt++);
    int param_cnt = 0;
    list<string>::iterator it;
    for(it = $3.result_list.begin(); it != $3.result_list.end(); it++) {
      sprintf(exp, "%sparam %s\n", exp, it->c_str());
    }
    sprintf(exp, "%svar %s\n%s = call %s\n", exp, tmp, tmp, iden.c_str());
    $$.code = $3.code + string(exp);
    $$.result = string(tmp);
  }
| Expression '[' Expression ']' {
    char exp[100];
    char tmp[10];
    sprintf(tmp, "t%d", tmp_var_cnt++);
    sprintf(exp, "var %s\n%s = %s [%s]\n", tmp, tmp, $1.result.c_str(), $3.result.c_str());
    $$.code = $1.code + $3.code + string(exp);
    $$.result = string(tmp);
  }
| Expression '+' Expression { TransBinaryOp($$, $1, $3, "+"); }
| Expression '-' Expression { TransBinaryOp($$, $1, $3, "-"); }
| Expression '*' Expression { TransBinaryOp($$, $1, $3, "*"); }
| Expression '/' Expression { TransBinaryOp($$, $1, $3, "/"); }
| Expression AND Expression { TransBinaryOp($$, $1, $3, "&&"); }
| Expression OR  Expression { TransBinaryOp($$, $1, $3, "||"); }
| Expression EQ  Expression { TransBinaryOp($$, $1, $3, "=="); }
| Expression NE  Expression { TransBinaryOp($$, $1, $3, "!="); }
| Expression LT  Expression { TransBinaryOp($$, $1, $3, "<"); }
| Expression GT  Expression { TransBinaryOp($$, $1, $3, ">"); }
;
ExpressionList
: Expression ExpressionAppend  {
    $$.result_list.clear();
    $$.code.clear();
    $$.result_list.push_back($1.result);
    $$.result_list.insert($$.result_list.end(), $2.result_list.begin(), $2.result_list.end());
    $$.code = $1.code + $2.code;
  }
| { $$.result_list.clear(); $$.code.clear(); }
;
ExpressionAppend
: ',' Expression ExpressionAppend {
    $$.result_list.clear();
    $$.code.clear();
    $$.result_list.push_back($2.result);
    $$.result_list.insert($$.result_list.end(), $3.result_list.begin(), $3.result_list.end());
    $$.code = $2.code + $3.code;
  }
| { $$.result_list.clear(); $$.code.clear(); }
;
Identifier
: ID { $$ = $1; }
;
%%


int main()
{
  root_env = new Env(NULL);
  last_env = NULL;
  this_env = root_env;
  ori_var_cnt = 0;
  tmp_var_cnt = 0;
  yyparse();
  return 0;
}
