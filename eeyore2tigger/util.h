#ifndef __UTIL_H__
 #define __UTIL_H__

#include <stdio.h>
#include <list>
#include <vector>
#include <string>
#include <map>
// #include <bitset>
#include <boost/dynamic_bitset.hpp>
using namespace std;

typedef struct Variable{
  string name;
  bool isGlobal;
  bool isArray;
  int arrayLength;
  int id;
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
  boost::dynamic_bitset<> varset;
}Sentence;

typedef list<Sentence> SentList;
class Function {
public:
  Function() { stack_size = 0; var_num = 0; }
  void set_name(string n) { name = n; }
  void set_param_num(int n) { param_num = n; }
  void set_sentlist(SentList s) { sentlist = s; }
  void addSentence(Sentence s) { sentlist.push_back(s); }
  void addVar(Variable v) {
    // varvec.push_back(v);
    v.id = var_num++;
    varmap.insert(make_pair(v.name, v));
  }
  void livenessAnalyze() {

  }
private:
  SentList sentlist;
  string name;
  int param_num;
  int stack_size;
  int var_num;
  map<string, Variable> varmap;
};

class Env{
public:
  void addVar(Variable v) {
    varvec.push_back(v);
  }
  void addFunc(Function f) {
    funclist.push_back(f);
  }
  void analyze() {
    list<Function>::iterator it_func;
    for(it_func = funclist.begin(); it_func != funclist.end(); it_func++) {
      int size = varvec.size();
      for(int i = 0; i < size; ++i) {
        it_func->addVar(varvec[i]);   // insert the global variables into the functions
      }
      it_func->livenessAnalyze();
    }

  }
private:
  list<Function> funclist;
  vector<Variable> varvec;    // global variables
};
#endif
