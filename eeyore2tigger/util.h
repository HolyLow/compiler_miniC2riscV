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
  void print_sentence() {
    string sentence; sentence.clear();
    switch (type) {
      case LABEL:
      sentence = "type LABAL ::: " + var1 + " :";
      break;
      case JUMP:
      sentence = "type JUMP ::: goto " + var1;
      break;
      case CON_JUMP:
      sentence = "type CON_JUMP ::: if "+var1+op+var2+" goto "+var3;
      break;
      case PARAM:
      sentence = "type PARAM ::: param "+var1;
      break;
      case RETURN:
      sentence = "type RETURN ::: return "+var1;
      break;
      case CALL:
      sentence = "type CALL ::: call "+var1;
      break;
      case CALL_ASSIGN:
      sentence = "type CALL_ASSIGN ::: "+var1+" = call "+var2;
      break;
      case OP_1:
      sentence = "type OP_1 ::: "+var1+" = "+op+var2;
      break;
      case OP_2:
      sentence = "type OP_2 ::: "+var1+" = "+var2+op+var3;
      break;
      case ASSIGN:
      sentence = "type ASSIGN ::: "+var1+" = "+var2;
      break;
      case LOAD:
      sentence = "type LOAD ::: "+var1+" = "+var2+"["+var3+"]";
      break;
      case STORE:
      sentence = "type STORE ::: "+var1+"["+var2+"] = "+var3;
      break;
    }
    printf("%s\n", sentence.c_str());
  }
  // bool use(string var);
  // bool define(string var);

}Sentence;

typedef list<Sentence> SentList;
class Function {
public:
  Function();
  void clear();
  void set_name(string n) { name = n; }
  void set_param_num(int n) {
    param_num = n;
    char param_name[10];
    for(int i = 0; i < n; ++i) {
      sprintf(param_name, "p%d", i);
      Variable var;
      var.name = (string)param_name;
      var.isGlobal = false;
      var.isArray = false;
      var.memoryAddr = stackAllocate(1);
      addVar(var);
    }
  }
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
  string stackAllocate(int size) {
    char str[20];
    sprintf(str, "%d", stack_size);
    stack_size += size;
    return (string)str;
  }
  void loadVar(string var_name, string& load_sent, string& reg_name);
  void storeVar(string var_name, string& store_sent, string& reg_name);
  string getTmpReg(int n = 1) {
    for(int i = 0; i < 8; ++i) {
      if(!a_reg_flag[i]) {
        n--;
        if(n <= 0) {
          char str[10];
          sprintf(str, "a%d", i);
          return (string)str;
        }
      }
    }
    check(n <= 0, "get tmp reg failed");
  }
  string getReg(string name) {
    int reg = varmap[name].reg;
    check(reg >= 0 && reg < reg_num, "invalid reg num");
    char regname[20];
    if(reg < 12) {
      sprintf(regname, "s%d", reg);
      s_reg_flag[reg] = true;
    }
    else {
      sprintf(regname, "t%d", reg-12);
      t_reg_flag[reg-12] = true;
    }
    return (string)regname;
  }


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
  void analyze() {
    // printf("begin analyze\n");
    list<Function>::iterator it_func;
    string code; code.clear();
    int size = varvec.size();
    for(int i = 0; i < size; ++i) {
      char global_var_name[10];
      sprintf(global_var_name, "v%d", i);
      varvec[i].memoryAddr = (string)global_var_name;
      if(varvec[i].isArray) {
        char array_length[10];
        sprintf(array_length, "%d", varvec[i].arrayLength);
        code += (string)global_var_name + " = malloc " + (string)array_length + "\n";
      }
      else {
        code += (string)global_var_name + " = 0\n";
      }
    }
    for(it_func = funclist.begin(); it_func != funclist.end(); it_func++) {
      for(int i = 0; i < size; ++i) {
        it_func->addVar(varvec[i]);   // insert the global variables into the functions
      }
      // printf("after it_func addvar\n");
      it_func->livenessAnalyze();
      it_func->registerAllocate();
      code += it_func->codeGenerate();
    }

    // printf("generated Tigger code is:\n\n");
    printf("%s\n", code.c_str());

  }
private:
  list<Function> funclist;
  vector<Variable> varvec;    // global variables
};
#endif
