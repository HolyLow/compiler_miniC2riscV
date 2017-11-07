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

int ori_var_cnt, tmp_var_cnt, label_cnt;

void StartNewEnv()
{
  last_env = this_env;
  this_env = new Env(last_env);
}

void RollBackEnv()
{
  this_env = last_env;
  last_env = this_env->parent();
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

Token* SearchToken(string id, bool isVar)
{
  return this_env->searchToken(id, isVar);
}

void TransUnaryOp(Expr & result, Expr & right, char *op)
{
  this_env->check(!isArray(right.type), "invalid usage on array variable of op", op);
  char exp[100];
  char tmp[10];
  sprintf(tmp, "t%d", tmp_var_cnt++);
  sprintf(exp, "var %s\n%s = %s %s\n", tmp, tmp, op, right.result.c_str());
  result.code = right.code + string(exp);
  result.result = string(tmp);
  result.type = right.type;
}

void TransBinaryOp(Expr & result, Expr & left, Expr & right, char *op)
{
  this_env->check(!isArray(left.type), "invalid usage on array variable of op", op);
  this_env->check(!isArray(right.type), "invalid usage on array variable of op", op);
  char exp[100];
  char tmp[10];
  sprintf(tmp, "t%d", tmp_var_cnt++);
  sprintf(exp, "var %s\n%s = %s %s %s\n", tmp, tmp, left.result.c_str(), op, right.result.c_str());
  result.code = left.code + right.code + string(exp);
  result.result = string(tmp);
  result.type = left.type;
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
%type <type> Type
/*%type <type> ParamDecl
%type <param_list> ParamDeclPack*/
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
  //printf("Goal recognized!\n");
  $$.code = $1.code + $2.code;
  printf("%s", $$.code.c_str());
}
;
GlobalD
: GlobalD VarDefn  { $$.code = $1.code + $2.code; }
| GlobalD FuncDefn { $$.code = $1.code + $2.code; }
| GlobalD FuncDecl { $$.code = $1.code; }
| { $$.code.clear(); }
;
MainFunc
: WORD_INT MAIN '(' ')' {
    VarList var_list; var_list.clear();
    AddFuncToken(string("main"), var_list, INT, true);
  }
  '{' { StartNewEnv(); }
  StatementPack '}' {
    RollBackEnv();
    char func_label[50];
    string iden = SearchToken(string("main"), false)->eeyore_name;
    sprintf(func_label, "%s [%d]\n", iden.c_str(), 0);
    char func_end[50];
    sprintf(func_end, "end f_main\n");
    $$.code = string(func_label) + $8.code + string(func_end);
  }
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
  StatementPack '}' {
    RollBackEnv();
    char func_label[50];
    string iden = SearchToken($2, false)->eeyore_name;
    sprintf(func_label, "%s [%d]\n", iden.c_str(), $4.size());
    char func_end[50];
    sprintf(func_end, "end %s\n", iden.c_str());
    $$.code = string(func_label) + $9.code + string(func_end);
  }
;
FuncDecl
: Type Identifier '(' ParamDefnPack ')' ';' { AddFuncToken($2, $4, $1, false); }
;
Type
: WORD_INT { $$ = INT; }
;
Statement
: '{' { StartNewEnv(); }
  StatementPack '}' { RollBackEnv(); $$.code = $3.code; }
| IF '(' Expression ')' Statement {
    this_env->check(!isArray($3.type), "invalid condition for if control", "");
    char label_out[10], exp[100], label_out_mark[10];
    sprintf(label_out, "l%d", label_cnt++);
    sprintf(label_out_mark, "%s:\n", label_out);
    sprintf(exp, "if %s == 0 goto %s\n", $3.result.c_str(), label_out);
    $$.code = $3.code + string(exp) + $5.code + string(label_out_mark);
  }
| IF '(' Expression ')' Statement ELSE Statement {
    this_env->check(!isArray($3.type), "invalid condition for if control", "");
    char label_out[10], exp[100], exp_else[50], label_out_mark[10];
    char label_else[10], label_else_mark[10];
    sprintf(label_else, "l%d", label_cnt++);
    sprintf(label_else_mark, "%s:\n", label_else);
    sprintf(label_out, "l%d", label_cnt++);
    sprintf(label_out_mark, "%s:\n", label_out);
    sprintf(exp, "if %s == 0 goto %s\n", $3.result.c_str(), label_else);
    sprintf(exp_else, "goto %s\n", label_out);
    $$.code = $3.code + string(exp) + $5.code + string(exp_else) + string(label_else_mark)
                      + $7.code + string(label_out_mark);
  }
| WHILE '(' Expression ')' Statement {
    this_env->check(!isArray($3.type), "invalid condition for if control", "");
    char label_out[10], exp[100], label_out_mark[10];
    char label_while[10], label_while_mark[10], exp_back[50];
    sprintf(label_while, "l%d", label_cnt++);
    sprintf(label_while_mark, "%s:\n", label_while);
    sprintf(label_out, "l%d", label_cnt++);
    sprintf(label_out_mark, "%s:\n", label_out);
    sprintf(exp, "if %s == 0 goto %s\n", $3.result.c_str(), label_out);
    sprintf(exp_back, "goto %s\n", label_while);
    $$.code = string(label_while_mark) + $3.code + string(exp) + $5.code
                                       + string(exp_back) + string(label_out_mark);
  }
| Identifier '=' Expression ';' {
    Token* token = SearchToken($1, true);
    this_env->check(!isArray(token->type), "invalid assignment to", token->minic_name);
    this_env->check(!isArray($3.type), "invalid assignment from array to non-array", "");
    string iden = token->eeyore_name;
    char exp[100];
    sprintf(exp, "%s = %s\n", iden.c_str(), $3.result.c_str());
    $$.code = $3.code + string(exp);
  }
| Identifier '[' Expression ']' '=' Expression ';' {
    Token* token = SearchToken($1, true);
    this_env->check(isArray(token->type), "invalid assignment to", token->minic_name);
    this_env->check(!isArray($3.type), "invalid index for array", "");
    this_env->check(!isArray($6.type), "invalid assignment from array to non-array", "");
    string iden = token->eeyore_name;
    char exp[100];
    char exp2[100], tmp[10];
    sprintf(tmp, "t%d", tmp_var_cnt++);
    sprintf(exp2, "var %s\n%s = 4 * %s\n", tmp, tmp, $3.result.c_str());
    sprintf(exp, "%s%s [%s] = %s\n", exp2, iden.c_str(), tmp, $6.result.c_str());
    $$.code = $3.code + $6.code + string(exp);
  }
| Identifier '(' ExpressionList ')' ';' {
    FuncToken* func_token = (FuncToken*)SearchToken($1, false);
    string iden = func_token->eeyore_name;
    char exp[500]; exp[0] = '\0';
    /*char tmp[10];
    sprintf(tmp, "t%d", tmp_var_cnt++);*/
    string param_code; param_code.clear();

    this_env->check($3.size() == func_token->param_list.size(),
                    "mismatched parameters for function",
                    func_token->minic_name.c_str());
    ExprList::iterator it_expr = $3.begin();
    ParamList::iterator it_param = func_token->param_list.begin();
    for(; it_expr != $3.end() && it_param != func_token->param_list.end(); it_expr++, it_param++) {
      this_env->check(it_expr->type == *it_param,
                      "mismatched parameters for function",
                      func_token->minic_name.c_str());
      sprintf(exp, "%sparam %s\n", exp, it_expr->result.c_str());
      param_code += it_expr->code;
    }

    /*sprintf(exp, "%svar %s\n%s = call %s\n", exp, tmp, tmp, iden.c_str());*/
    sprintf(exp, "%scall %s\n", exp, iden.c_str());
    $$.code = param_code + string(exp);
    /*$$.result = string(tmp);
    $$.type = func_token->return_type;*/
  }
| VarDefn { $$.code = $1.code; }
| RETURN Expression ';' {
  char exp[100];
  sprintf(exp, "return %s\n", $2.result.c_str());
  $$.code = $2.code + string(exp);
}
;
StatementPack
: StatementPack Statement { $$.code = $1.code + $2.code; }
| { $$.code.clear(); }
;
Expression
: NUM { $$.result = string($1); $$.code.clear(); $$.type = INT; }
| Identifier {
     Token* token = SearchToken($1, true);
    $$.result = token->eeyore_name;
    $$.code.clear();
    $$.type = token->type;
    /*printf("input name is %s, searched name is %s,  type is %s\n",
           $1.c_str(), token->minic_name.c_str(), strType($$.type).c_str());*/
  }
| '-' Expression %prec UMINUS { TransUnaryOp($$, $2, "-"); }
| '!' Expression              { TransUnaryOp($$, $2, "!"); }
| Identifier '(' ExpressionList ')' {
    FuncToken* func_token = (FuncToken*)SearchToken($1, false);
    string iden = func_token->eeyore_name;
    char exp[500]; exp[0] = '\0';
    char tmp[10];
    sprintf(tmp, "t%d", tmp_var_cnt++);
    string param_code; param_code.clear();

    this_env->check($3.size() == func_token->param_list.size(),
                    "mismatched parameters for function",
                    func_token->minic_name.c_str());
    ExprList::iterator it_expr = $3.begin();
    ParamList::iterator it_param = func_token->param_list.begin();
    for(; it_expr != $3.end() && it_param != func_token->param_list.end(); it_expr++, it_param++) {
      this_env->check(it_expr->type == *it_param,
                      "mismatched parameters for function",
                      func_token->minic_name.c_str());
      sprintf(exp, "%sparam %s\n", exp, it_expr->result.c_str());
      param_code += it_expr->code;
    }

    sprintf(exp, "%svar %s\n%s = call %s\n", exp, tmp, tmp, iden.c_str());
    $$.code = param_code + string(exp);
    $$.result = string(tmp);
    $$.type = func_token->return_type;
  }
| Expression '[' Expression ']' {
    this_env->check(isArray($1.type), "invalid usage of non_array variable", "");
    this_env->check(!isArray($3.type), "invalid usage of array variable", "");
    string iden = $1.result;
    char exp[100];
    char tmp1[10], tmp2[10];
    sprintf(tmp1, "t%d", tmp_var_cnt++);
    sprintf(tmp2, "t%d", tmp_var_cnt++);
    char exp2[100];
    sprintf(exp2, "var %s\n%s = 4 * %s\n", tmp1, tmp1, $3.result.c_str());
    sprintf(exp, "%svar %s\n%s = %s [%s]\n", exp2, tmp2, tmp2, iden.c_str(), tmp1);
    $$.code = $1.code + $3.code + string(exp);
    $$.result = string(tmp2);
    $$.type = deArray($1.type);
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
    $$.clear();
    $$.push_back($1);
    $$.insert($$.end(), $2.begin(), $2.end());
  }
| { $$.clear(); }
;
ExpressionAppend
: ',' Expression ExpressionAppend {
    $$.clear();
    $$.push_back($2);
    $$.insert($$.end(), $3.begin(), $3.end());
  }
| { $$.clear(); }
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
  label_cnt = 0;
  yyparse();
  return 0;
}
