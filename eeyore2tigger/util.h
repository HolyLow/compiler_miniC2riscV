#ifndef __UTIL_H__
 #define __UTIL_H__

#include <stdio.h>
#include <list>
#include <vector>
#include <string>
using namespace std;

typedef struct Variable{
  string name;
  bool isGlobal;
  bool isArray;
  int arrayLength;
}Variable;

typedef enum SentenceType {
  LABEL,
  JUMP,
  CON_JUMP,
  PARAM,
  RETURN,
  CALL,
  CALL_ASSIGN,
  OP_1,
  OP_2,
  ASSIGN,
  LOAD,
  STORE
}SentenceType;

typedef struct Sentence{
  string var1, var2, var3;
  string op;
  SentenceType type;
}Sentence;

typedef list<Sentence> SentList;
typedef struct Function{
  SentList sentlist;
  string name;
  int param_num;
  int stack_size;
  Function() { stack_size = 0; }
}Function;

class Env{
public:
  void addVar(Variable v) {
    varvec.push_back(v);
  }
  void addFunc(Function f) {
    funclist.push_back(f);
  }
  void analyze() {
    
  }
private:
  list<Function> funclist;
  vector<Variable> varvec;
};
#endif
