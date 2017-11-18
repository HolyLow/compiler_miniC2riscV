#ifndef __UTIL_H__
 #define __UTIL_H__

#include <stdio.h>
#include <list>
#include <vector>
#include <string>
using namespace std;
typedef enum SentenceType {
  LABEL,
  JUMP,
  CON_JUMP,
  PARAM,
  RETURN,
  CALL,
  OP_1,
  OP_2,
  ASSIGN,
  OP_ARRAY
}SentenceType;

typedef struct Sentence{
  string var1, var2, var3;
  string op;
  SentenceType type;
}Sentence;

class Function{
public:


private:
  list<Sentence> sentlist;
  string name;
  int param_num;
  int stack_size;
};

class Env{
public:

private:
  list<Function> funclist;
};
#endif
