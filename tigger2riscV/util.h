#ifndef ___UTIL_H___
 #define ___UTIL_H___

#include <stdio.h>
#include <stdlib.h>
#include <string>

typedef enum Operation{
  ADD,
  SUBTRACT,
  MULTIPLY,
  DIVIDE,
  MOD,
  LT,
  GT,
  LE,
  GE,
  NE,
  EQ,
  AND,
  OR,
  NOT,
  NEG
} Operation;

using namespace std;

#endif
