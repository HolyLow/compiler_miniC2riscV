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
  int reg;
  bool isOverflowed;
  bool isLoaded;
  Variable() {
    isOverflowed = false;
    isLoaded = false;
  }

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
  void print_sentence();

}Sentence;

typedef list<Sentence> SentList;
class Function {
public:
  Function();
  void clear();
  void set_name(string n) { name = n; }
  void set_param_num(int n);
  void set_sentlist(SentList s) { sentlist = s; }
  void addSentence(Sentence s) { sentlist.push_back(s); }
  void addVar(Variable v);

  void livenessAnalyze();
  void registerAllocate();
  string codeGenerate();
private:
  string name;
  int param_num;
  int stack_size;
  int var_num;
  int reg_num;
  vector<int> var_live_length;
  bool s_reg_flag[12];
  bool t_reg_flag[7];
  bool a_reg_flag[8];
  string s_reg_memoryAddr[12];
  string t_reg_memoryAddr[7];
  SentList sentlist;
  map<string, Variable> varmap;
  vector<string> id2name;

  void overflow(SentList::iterator it_this, int var_id);
  string stackAllocate(int size);
  void loadVar(string var_name, string& load_sent, string& reg_name);
  void storeVar(string var_name, string& store_sent, string& reg_name);
  string getTmpReg(int n = 1);
  string getReg(string name);


  static bool isVar(string v) {
    char c = v.c_str()[0];
    return (c == 'p' || c == 'T' || c == 't');
  }
  static int idParameter(string v) {
    if(v.c_str()[0] != 'p')
      return -1;
    return atoi(&v.c_str()[1]);
  }
  static void check(bool flag, string msg) {
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
  void analyze();
private:
  list<Function> funclist;
  vector<Variable> varvec;    // global variables
};
#endif
