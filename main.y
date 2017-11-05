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

void AddVarToken(string id, TokenType type)
{
  this_env->insertVarToken(id, type);
}

void AddFuncToken(string id, ParamList param_list, TokenType return_type, bool isDef)
{
  this_env->insertFuncToken(id, param_list, return_type, isDef);
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
%type <type> Type VarDecl
%type <param_list> VarDeclPack
%type <part> Goal GlobalD VarDefn StatementPack ExpressionList ExpressionAppend


%token IF ELSE WHILE RETURN MAIN
%token WORD_INT
%token <str>  ID
%token <num> NUM
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
: WORD_INT { printf("before main\n"); }
  MAIN { printf("after main\n"); }
  '(' ')' '{' { StartNewEnv(); }
  StatementPack '}' { RollBackEnv(); }
;
VarDefn
: Type Identifier ';' { AddVarToken($2, $1); }
| Type Identifier '[' NUM ']' ';' { AddVarToken($2, Array($1)); }
;
VarDecl
: Type Identifier { $$ = $1; }
| Type Identifier '[' NUM ']' { $$ = Array($1); }
;
VarDeclPack
: VarDeclPack VarDecl { $$ = $1; $$.insert($$.end(), $2); }
| { $$.clear(); }
;
FuncDefn
: Type Identifier '(' VarDeclPack ')'  { AddFuncToken($2, $4, $1, true); }
  '{' { StartNewEnv(); }
  StatementPack '}' { RollBackEnv(); }
;
FuncDecl
: Type Identifier '(' VarDeclPack ')' ';' { AddFuncToken($2, $4, $1, false); }
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
| Expression OR  Expression
| Expression EQ  Expression
| Expression NE  Expression
| Expression LT  Expression
| Expression GT  Expression
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
: ID { $$ = $1; }
;
%%


int main()
{
  root_env = new Env(NULL);
  last_env = NULL;
  this_env = root_env;
  yyparse();
  return 0;
}
