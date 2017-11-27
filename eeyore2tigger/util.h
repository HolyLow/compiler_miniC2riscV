#ifndef __UTIL_H__
 #define __UTIL_H__

#include <stdio.h>
#include <stdlib.h>
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
  string memoryAddr;
  string reg;
  bool isOverflowed;
  bool isLoaded;

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
  // bool use(string var);
  // bool define(string var);

}Sentence;

typedef list<Sentence> SentList;
class Function {
public:
  Function();
  void clear();
  void set_name(string n) { name = n; }
  void set_param_num(int n) { param_num = n; }
  void set_sentlist(SentList s) { sentlist = s; }
  void addSentence(Sentence s) { sentlist.push_back(s); }
  void addVar(Variable v);

  void livenessAnalyze();
  void registerAllocate();
private:
  SentList sentlist;
  string name;
  int param_num;
  int stack_size;
  int var_num;
  int reg_num;
  vector<int> var_live_length;
  bool s_reg_flag[12];
  bool t_reg_flag[7];
  map<string, Variable> varmap;
  vector<string> id2name;
  map<string, SentList::reverse_iterator> labelmap;

  void overflow(SentList::iterator it_this, int var_id);
  string stackAllocate(int size) {
    char str[20];
    sprintf(str, "%d", stack_size+1);
    stack_size += size;
    return (string)str;
  }
  bool isVar(string v) {
    char c = v.c_str()[0];
    return (c == 'p' || c == 'T' || c == 't');
  }
  void check(bool flag, string msg) {
    if(!flag) {
      printf("error: %s\n", msg.c_str());
      exit(1);
    }
  }
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
      it_func->registerAllocate();
    }

  }
private:
  list<Function> funclist;
  vector<Variable> varvec;    // global variables
};
#endif
