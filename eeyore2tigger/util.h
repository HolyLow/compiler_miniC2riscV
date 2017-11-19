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
  /* attentions! no label checking, no num-or-variable checking */
  void livenessAnalyze() {
    SentList::reverse_iterator rit_this;
    for(rit_this = sentlist.rbegin(); rit_this != sentlist.rend(); rit_this++) {
      rit_this->varset.resize(var_num);
      if(rit_this->type == LABEL) {
        labelmap.insert(make_pair(rit_this->var1, rit_this));
      }
    }
    int bit_cnt = 0, bit_begin = 0, bit_end = 0;
    boost::dynamic_bitset<> last_varset;
    last_varset.resize(var_num);
    boost::dynamic_bitset<> this_varset;
    this_varset.resize(var_num);
    SentList::reverse_iterator rit_last, rit_jump;
    do {
      bit_cnt = 0;
      for(rit_this = sentlist.rbegin(); rit_this != sentlist.rend(); rit_this++) {
        if(rit_this == sentlist.rbegin())
          last_varset.reset();
        else {
          rit_last = rit_this; rit_last--;
          last_varset = rit_last->varset;
        }
        this_varset = rit_this->varset;
        bit_begin = rit_this->varset.count();
        switch (rit_this->type) {
          case LABEL:
          case CALL:
            this_varset = last_varset;
            break;
          case JUMP:
            this_varset = labelmap[rit_this->var1]->varset;
            break;
          case CON_JUMP:    // always assume that it is compared to 0, like "if t1 > 0 goto l1"
            this_varset = last_varset;
            this_varset |= labelmap[rit_this->var3]->varset;
            this_varset.set(varmap[rit_this->var1].id);
            break;
          case PARAM:
          case RETURN:
            this_varset = last_varset;
            this_varset.set(varmap[rit_this->var1].id);
            break;
          case CALL_ASSIGN:
            this_varset = last_varset;
            this_varset.reset(varmap[rit_this->var1].id);
            break;
          case OP_1:
            this_varset = last_varset;
            this_varset.reset(varmap[rit_this->var1].id);
            this_varset.set(varmap[rit_this->var2].id);
            break;
          case OP_2:
            this_varset = last_varset;
            this_varset.reset(varmap[rit_this->var1].id);
            this_varset.set(varmap[rit_this->var2].id);
            this_varset.set(varmap[rit_this->var3].id);
            break;
          case ASSIGN:    // assume that only assin sentences and con_jump sentences will accur integer
            this_varset = last_varset;
            this_varset.reset(varmap[rit_this->var1].id);
            if(isVar(rit_this->var2))
              this_varset.set(varmap[rit_this->var2].id);
            break;
          case LOAD:
            this_varset = last_varset;
            this_varset.reset(varmap[rit_this->var1].id);
            this_varset.set(varmap[rit_this->var3].id);
            break;
          case STORE:
            this_varset = last_varset;
            this_varset.set(varmap[rit_this->var2].id);
            this_varset.set(varmap[rit_this->var3].id);
            break;
          default:
            check(false, "sentence type not defined");
        }
        bit_end = this_varset.count();
        rit_this->varset = this_varset;
        check(bit_end >= bit_begin, "live variable num reduced after updating");
        bit_cnt += (bit_end - bit_begin);
      }
    }while(bit_cnt > 0);

  }
private:
  SentList sentlist;
  string name;
  int param_num;
  int stack_size;
  int var_num;
  map<string, Variable> varmap;
  map<string, SentList::reverse_iterator> labelmap;
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
    }

  }
private:
  list<Function> funclist;
  vector<Variable> varvec;    // global variables
};
#endif
