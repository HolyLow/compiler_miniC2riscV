#ifndef _NODE_H_
 #define _NODE_H_
#include "util.h"
#include <string>
using namespace std;


struct Part {
  string code;
};


struct Expr {
  string code;

};

struct Func {
  string code;
};

struct Stat {
  string code;
};

struct Iden {
  string code;
};

typedef struct Nodes {
  struct Expr expr;
  struct Func func;
  struct Stat stat;
  string      iden;
  struct Part part;
  TokenType   type;
  ParamList   param_list;
  char * str;
  int    num;
} Nodes;


#endif
