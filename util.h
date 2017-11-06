#ifndef _UTIL_H_
 #define _UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <string>
#include <list>
using namespace std;
// enum RegType { TEMP, ORIGIN, NOTSET = 0 };
// struct Register {
//   RegType type;
//   int     id;
//   string  name;
// };

typedef enum TokenType{ FUNC, INT, INT_ARRAY } TokenType;
typedef list<TokenType> ParamList;
inline TokenType Array(TokenType t)
{
  if (t == INT)
    return INT_ARRAY;
  return t;
}

struct FuncToken {
  string minic_name;
  string eeyore_name;
  ParamList param_list;
  TokenType return_type;
  bool isDefined;
  FuncToken() {}
  FuncToken(string name, ParamList paramlist, TokenType ty, bool isDef) {
    minic_name = name;
    param_list = paramlist;
    return_type = ty;
    isDefined = isDef;
    eeyore_name = string("f_") + minic_name;
  }
};

struct VarToken {
  TokenType type;
  string minic_name;
  string eeyore_name;
  VarToken() {}
  VarToken(string name, TokenType ty, string ename) {
    minic_name = name;
    type = ty;
    eeyore_name = ename;
  }
};

typedef list<VarToken> VarList;
// struct Token {
//   string    name;
//   TokenType type;
//   int       param;          // for FUNC, this is the num of func parameters;
//   // for INT_ARRAY, this is the num of elements
//   list<TokenType> varlist;
//   Register  reg;
//   int       id;
//   string    eeyore_name;
//
//   Token(string str, TokenType t, int n = 0, Register r = NOTSET, int i = 0) {
//     name = str; type = t; param = n; reg = r; id = i;
//   }
//
//
// };

class Env {
public:
  Env (Env *p = NULL) {
    _parent = p;
    functoken.clear();
    vartoken.clear();
  }
  Env* parent() {
    return _parent;
  }
  bool findToken(string name) {
    map<string, VarToken>::iterator it_var = vartoken.find(name);
    map<string, FuncToken>::iterator it_func = functoken.find(name);
    return (it_var != vartoken.end() || it_func != functoken.end());
  }
  void insertVarToken(string name, TokenType type, string ename) {
    if(findToken(name)) {
      char msg[100];
      sprintf(msg, "duplicated definition of %s", name.c_str());
      error(msg);
    }
    vartoken[name] = VarToken(name, type, ename);
  }
/* if a function is not found declared or defined, simply insert it with the isDef
 * tag to declare or define it;
 * if a function is declared but not defined, then check and set the isDefined tag;
 *
 */
  void insertFuncToken(string name, ParamList param_list, TokenType return_type, bool isDef) {
    if(!findToken(name)) {
      functoken[name] = FuncToken(name, param_list, return_type, isDef);
    }

    else {
      map<string, FuncToken>::iterator it_func = functoken.find(name);
      if(it_func != functoken.end() && vartoken.find(name) == vartoken.end()
        && !it_func->second.isDefined && isDef
        && it_func->second.param_list == param_list && it_func->second.return_type == return_type)
      {
        it_func->second.isDefined = true;
      }

      else {
        char msg[100];
        sprintf(msg, "duplicated definition of %s", name.c_str());
        error(msg);
      }
    }
  }
  string searchToken(string name, bool isVar) {
    if(isVar) {
      map<string, VarToken>::iterator it = vartoken.find(name);
      if(it == vartoken.end()) {
        if(_parent == NULL) {
          char msg[100];
          sprintf(msg, "undefined variable %s", name.c_str());
          error(msg);
        }
        else return _parent->searchToken(name, isVar);
      }
      else return it->second.eeyore_name;
    }
    else {
      map<string, FuncToken>::iterator it = functoken.find(name);
      if(it == functoken.end()) {
        if(_parent == NULL) {
          char msg[100];
          sprintf(msg, "undefined variable %s", name.c_str());
          error(msg);
        }
        else return _parent->searchToken(name, isVar);
      }
      else return it->second.eeyore_name;
    }
  }

private:
  class Env* _parent;
  map<string, FuncToken> functoken;
  map<string, VarToken>  vartoken;
  static void error(char *msg) {
    printf("error: %s\n", msg);
    exit(1);
  }
};


#endif
