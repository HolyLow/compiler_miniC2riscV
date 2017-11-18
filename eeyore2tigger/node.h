#ifndef __NODE_H__
 #define __NODE_H__
#include "util.h"
typedef struct Nodes {
  char * str;
  string part;
  Sentence sent;
  Variable var;
  SentList sentlist;
  Function func;
  int num;
} Nodes;
#endif
