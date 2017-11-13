#ifndef _NODE_H_
 #define _NODE_H_
#include "util.h"
#include <string>
using namespace std;


typedef struct Part {
  string code;
}Part;


typedef struct Expr {
  string      code;
  string      result;
  TokenType   type;
  Expr() { type = INT; }
}Expr;
typedef list<Expr> ExprList;

typedef struct Func {
  string code;
}Func;

typedef struct Stat {
  string code;
}Stat;

typedef struct Iden {
  string code;
}Iden;

// typedef struct ExprList {
//   string code;
//   list<string> result_list;
// }ExprList;

typedef struct Nodes {
  Expr        expr;
  Func        func;
  Stat        stat;
  string      iden;
  Part        part;
  TokenType   type;
  // ParamList   param_list;
  VarToken    var_info;
  VarList     var_list;
  ExprList    expr_list;
  char * str;
  // int    num;
} Nodes;


#endif
