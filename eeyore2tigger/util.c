#include "util.h"
#include <string.h>

/*
switch(type) {
case LABEL:
case JUMP:
case CON_JUMP:
case PARAM:
case RETURN:
case CALL:
case CALL_ASSIGN:
case OP_1:
case OP_2:
case ASSIGN:
case LOAD:
case STORE:

}
*/

void Sentence::print_sentence() {
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
Function::Function()  {
  stack_size = 0;
  var_num = 0;
  reg_num = 19;
  memset(s_reg_flag, 0, sizeof(s_reg_flag));
  memset(t_reg_flag, 0, sizeof(t_reg_flag));
  memset(a_reg_flag, 0, sizeof(a_reg_flag));

  sentlist.clear();
  varmap.clear();
  id2name.clear();

  for(int i = 0; i < 12; ++i)
    s_reg_memoryAddr[i].clear();
  for(int i = 0; i < 7; ++i)
    t_reg_memoryAddr[i].clear();
}

void Function::clear() {
  stack_size = 0;
  var_num = 0;
  reg_num = 19;
  memset(s_reg_flag, 0, sizeof(s_reg_flag));
  memset(t_reg_flag, 0, sizeof(t_reg_flag));
  memset(a_reg_flag, 0, sizeof(a_reg_flag));

  sentlist.clear();
  varmap.clear();
  id2name.clear();

  for(int i = 0; i < 12; ++i)
    s_reg_memoryAddr[i].clear();
  for(int i = 0; i < 7; ++i)
    t_reg_memoryAddr[i].clear();
}

void Function::set_param_num(int n) {
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

void Function::addVar(Variable v) {
  v.id = var_num++;
  if(v.isArray && !v.isGlobal) {
    v.memoryAddr = stackAllocate(v.arrayLength/4);
  }
  varmap[v.name] = v;
  id2name.push_back(v.name);
  // printf("id2name id is %d, v id is %d\n", id2name.size(), v.id);
  check(id2name[v.id] == v.name, "wrong id2name");
}

/* attention! no label checking, no num-or-variable checking */
void Function::livenessAnalyze() {
  // printf("varmap size is %d, id2name size is %d, var_num is %d\n", varmap.size(), id2name.size(), var_num);
  // printf("sentlist size is %d\n", sentlist.size());
  SentList::reverse_iterator rit_this;
  map<string, SentList::reverse_iterator> labelmap;
  // printf("var num is %d\n", var_num);
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
  SentList::reverse_iterator rit_last;
  do {
    bit_cnt = 0;
    // int line_cnt = 0;
    for(rit_this = sentlist.rbegin(); rit_this != sentlist.rend(); rit_this++) {
      if(rit_this == sentlist.rbegin())
        last_varset.reset();
      else {
        rit_last = rit_this; rit_last--;
        last_varset = rit_last->varset;
      }
      this_varset = rit_this->varset;
      bit_begin = rit_this->varset.count();
      // line_cnt++;
      // printf("line_cnt = %d, begin line\n", line_cnt);
      // rit_this->print_sentence();
      switch (rit_this->type) {
        case LABEL:
        case CALL:
          this_varset = last_varset;
          break;
        case JUMP:
          this_varset = labelmap[rit_this->var1]->varset;
          break;
        case CON_JUMP:
        // always assume that it is compared to 0, like "if t1 > 0 goto l1"
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
        case ASSIGN:
        // assume that only assign sentences and con_jump sentences will accur integer
          this_varset = last_varset;
          this_varset.reset(varmap[rit_this->var1].id);
          if(isVar(rit_this->var2))
            this_varset.set(varmap[rit_this->var2].id);
          break;
        case LOAD:
          this_varset = last_varset;
          this_varset.reset(varmap[rit_this->var1].id);
          this_varset.set(varmap[rit_this->var2].id);
          this_varset.set(varmap[rit_this->var3].id);
          break;
        case STORE:
          this_varset = last_varset;
          this_varset.set(varmap[rit_this->var1].id);
          this_varset.set(varmap[rit_this->var2].id);
          this_varset.set(varmap[rit_this->var3].id);
          break;
        default:
          check(false, "sentence type not defined");
      }
      // printf("line_cnt = %d, end line\n\n", line_cnt);
      bit_end = this_varset.count();
      rit_this->varset = this_varset;
      check(bit_end >= bit_begin, "live variable num reduced after updating");
      bit_cnt += (bit_end - bit_begin);
    }
  }while(bit_cnt > 0);

  // printf("fine, after dotted liveness\n");
  /* connect the dotted liveness lines for every variable */
  for(rit_this = sentlist.rbegin(); rit_this != sentlist.rend(); rit_this++) {
    rit_last = rit_this; rit_last++;
    if(rit_last == sentlist.rend())
      break;
    last_varset = rit_last->varset;
    this_varset = rit_this->varset;
    for(int i = 0; i < var_num; ++i) {
      if(this_varset[i] && !last_varset[i]) {
        for(; rit_last != sentlist.rend() && !rit_last->varset[i]; rit_last++);
        if(rit_last != sentlist.rend()) {
          for(; rit_last != rit_this; rit_last--)
            rit_last->varset.set(i);
        }
      }
    }
  }

  SentList::iterator it_this, it_next, it_next_after;
  int line_cnt = 0;
  var_last_live.resize(var_num);
  for(int i = 0; i < var_num; ++i)
    var_last_live[i] = 10000000;
  for(it_this = sentlist.begin(); it_this != sentlist.end(); it_this++, line_cnt++) {
    it_next = it_this; it_next++;
    if(it_next == sentlist.end())
      break;
    it_next_after = it_next; it_next_after++;
    for(int i = 0; i < var_num; ++i) {
      // // to eliminate the double assign, such as making "t0 = 1; T0 = t0" to become "T0 = 1"
      // if(!it_this->varset[i] && it_next->varset[i]) {
      //   if(it_next_after == sentlist.end() || !it_next_after->varset[i]) {
      //     if(it_this->type == ASSIGN && it_next->type == ASSIGN) {
      //       check(it_this->var1 == it_next->var2, "invalid assign deletion");
      //       it_this->var1 = it_next->var1;
      //       // it_this->varset = it_this->varset & it_next->varset;
      //       it_next = sentlist.erase(it_next);
      //       break;
      //     }
      //   }
      // }

      // use the die-place of the variables to help decide which to be overflowed
      if(it_this->varset[i] && !it_next->varset[i]) {
        var_last_live[i] = line_cnt;
      }
    }
    // check if this sentense defines a variable which is never used, if it is, eliminate it
    if((it_this->type == ASSIGN || it_this->type == OP_1 || it_this->type == OP_2)
       && it_next != sentlist.end() && !it_next->varset[varmap[it_this->var1].id]) {
        it_this = sentlist.erase(it_this);
        it_this--;
    }
  }
}

void Function::registerAllocate() {
  SentList::iterator it_this, it_last, it_next;
  for(it_this = sentlist.begin(); it_this != sentlist.end(); it_this++) {
    while(it_this->varset.count() > reg_num) {
      int max_live = 0, max_i = 0;
      for(int i = 0; i < var_num; ++i) {
        if(it_this->varset[i] && max_live < var_last_live[i]) {
          max_live = var_last_live[i];
          max_i = i;
        }
      }
      overflow(it_this, max_i);
    }
  }

  vector<bool> var_assigned;
  vector<bool> reg_assigned;
  var_assigned.resize(var_num);
  for(int i = 0; i < var_num; ++i)
    var_assigned[i] = false;
  reg_assigned.resize(reg_num);
  for(int i = 0; i < reg_num; ++i)
    reg_assigned[i] = false;

  for(it_this = sentlist.begin(); it_this != sentlist.end(); it_this++) {
    it_next = it_this; it_next++;
    if(it_next != sentlist.end()) {
      for(int i = 0; i < var_num; ++i) {
        if(it_this->varset[i] && !it_next->varset[i] && !var_assigned[i]) {
          int reg_cnt = 0;
          for(; reg_cnt < reg_num && reg_assigned[reg_cnt]; reg_cnt++);
          check(reg_cnt < reg_num, "unchecked overflow");
          varmap[id2name[i]].reg = reg_cnt;
          reg_assigned[reg_cnt] = true;
          var_assigned[i] = true;
        }
      }
      /* assign for all live but not assigned variables */
      int reg_cnt = 0;
      for(int i = 0; i < var_num; ++i) {
        if(it_this->varset[i] && !var_assigned[i]) {
          while((reg_cnt<reg_num) && reg_assigned[reg_cnt])
            reg_cnt++;
          check(reg_cnt < reg_num, "unchecked overflow");
          varmap[id2name[i]].reg = reg_cnt;
          reg_assigned[reg_cnt] = true;
          var_assigned[i] = true;
        }
      }
      /* end of DU-Chain, release related register */
      for(int i = 0; i < var_num; ++i) {
        if(it_this->varset[i] && !it_next->varset[i]) {
          check(var_assigned[i], "release before assign");
          reg_assigned[varmap[id2name[i]].reg] = false;
        }
      }
    }
  }
  for(int i = 0; i < var_num; ++i) {
    if(!var_assigned[i])
      varmap[id2name[i]].reg = -1;
  }
}

string Function::codeGenerate() {
  string code;
  string reg1, reg2, reg3;
  string load1, load2, load3;
  string store1, store2, store3;
  SentList::iterator it_this;
  string line; line.clear();
  check(param_num <= 8, "too many parameters");

  string offset;
  SentList::iterator it_param;
  for(it_this = sentlist.begin(); it_this != sentlist.end(); it_this++) {
    switch(it_this->type) {
      case LABEL:
        line = it_this->var1 + ":\n";
        code += line;
        break;
      case JUMP:
        line = "goto " + it_this->var1 + "\n";
        code += line;
        break;
      case CON_JUMP:
        check(it_this->var2 == "0", "invalid con_jump");
        loadVar(it_this->var1, load1, reg1);
        line = "if " + reg1 + " " + it_this->op + " x0 goto " + it_this->var3;
        line += '\n';
        code += (load1 + line);
        break;
      case PARAM:
        it_param = it_this;
        for(int cnt = 0; it_param->type == PARAM; cnt++, it_param++) {
          check(cnt < 8, "doesn't support more than 8 parameters\n");
          loadVar(it_param->var1, load1, reg1);
          line.clear();
          char param_reg[10];
          sprintf(param_reg, "a%d", cnt);
          if(reg1 != (string)param_reg) {
            line = (string)param_reg + " = " + reg1 + "\n";
          }
          a_reg_flag[cnt] = true;
          code += load1 + line;
        }
        break;
      case RETURN:
        loadVar(it_this->var1, load1, reg1);
        line.clear();
        if(reg1 != "a0") {
          line = "a0 = " + reg1 + "\n";
        }
        for(int i = 0; i < 12; ++i) {
          char reg_name[10];
          if(s_reg_flag[i]) {
            if(s_reg_memoryAddr[i].length() == 0)
              s_reg_memoryAddr[i] = stackAllocate(1);
            sprintf(reg_name, "s%d", i);
            line += "load " + s_reg_memoryAddr[i] + " " + (string)reg_name + "\n";
          }
        }
        line += "return\n";
        code += load1 + line;
        break;
      case CALL:
        line.clear();
        for(int i = 0; i < 7; ++i) {
          char reg_name[10];
          if(t_reg_flag[i]) {
            if(t_reg_memoryAddr[i].length() == 0)
              t_reg_memoryAddr[i] = stackAllocate(1);
            sprintf(reg_name, "t%d", i);
            line += "store " + (string)reg_name + " " + t_reg_memoryAddr[i] + "\n";
          }
        }
        memset(a_reg_flag, 0, sizeof(a_reg_flag));
        line += "call " + it_this->var1 + "\n";
        for(int i = 0; i < 7; ++i) {
          char reg_name[10];
          if(t_reg_flag[i]) {
            check(t_reg_memoryAddr[i].length() != 0, "unallocated t_reg");
            sprintf(reg_name, "t%d", i);
            line += "load " + t_reg_memoryAddr[i] + " " + (string)reg_name + "\n";
          }
        }
        code += line;
        break;
      case CALL_ASSIGN:
        line.clear();
        for(int i = 0; i < 7; ++i) {
          char reg_name[10];
          if(t_reg_flag[i]) {
            if(t_reg_memoryAddr[i].length() == 0)
              t_reg_memoryAddr[i] = stackAllocate(1);
            sprintf(reg_name, "t%d", i);
            line += "store " + (string)reg_name + " " + t_reg_memoryAddr[i] + "\n";
          }
        }
        memset(a_reg_flag, 0, sizeof(a_reg_flag));
        storeVar(it_this->var1, store1, reg1);
        line += "call " + it_this->var2 + "\n";
        if(reg1 != "a0")
          line += reg1 + " = " + "a0\n";
        for(int i = 0; i < 7; ++i) {
          char reg_name[10];
          if(t_reg_flag[i]) {
            check(t_reg_memoryAddr[i].length() != 0, "unallocated t_reg");
            sprintf(reg_name, "t%d", i);
            line += "load " + t_reg_memoryAddr[i] + " " + (string)reg_name + "\n";
          }
        }
        code += line + store1;
        break;
      case OP_1:
        loadVar(it_this->var2, load2, reg2);
        storeVar(it_this->var1, store1, reg1);
        line = reg1 + " = " + it_this->op + " " + reg2 + "\n";
        code += load2 + line + store1;
        break;
      case OP_2:
        loadVar(it_this->var2, load2, reg2);
        if(it_this->var2 != it_this->var3) {
          loadVar(it_this->var3, load3, reg3);
          storeVar(it_this->var1, store1, reg1);
          line = reg1 + " = " + reg2 + " " + it_this->op + " " + reg3 + "\n";
          code += load2 + load3 + line + store1;
        }
        else {
          storeVar(it_this->var1, store1, reg1);
          line = reg1 + " = " + reg2 + " " + it_this->op + " " + reg2 + "\n";
          code += load2 + line + store1;
        }
        break;
      case ASSIGN:
        if(isVar(it_this->var2)) {
          loadVar(it_this->var2, load2, reg2);
          storeVar(it_this->var1, store1, reg1);
          line.clear();
          if(reg1 != reg2)
            line += reg1 + " = " + reg2 + "\n";
          code += load2 + line + store1;
        }
        else {
          storeVar(it_this->var1, store1, reg1);
          line = reg1 + " = " + it_this->var2 + "\n";
          code += line + store1;
        }
        break;
      case LOAD:
        loadVar(it_this->var2, load2, reg2);
        loadVar(it_this->var3, load3, reg3);
        storeVar(it_this->var1, store1, reg1);
        offset = getTmpReg(2);
        line = offset + " = " + reg3 + " + " + reg2 + "\n";
        line += reg1 + " = " + offset + "[0]\n";
        code += load2 + load3 + line + store1;
        break;
      case STORE:
        loadVar(it_this->var1, load1, reg1);
        loadVar(it_this->var2, load2, reg2);
        loadVar(it_this->var3, load3, reg3);
        offset = getTmpReg(2);
        line = offset + " = " + reg1 + " + " + reg2 + "\n";
        line += offset + "[0] = " + reg3 + "\n";
        code += load1 + load2 + load3 + line;
        break;
      default:
        check(false, "undefined sentence type");
    }
  }
  /* in the front of the code, we have to push all used s_registers to stack,  */
  /* and push all overflowed parameters into the stack */
  /* and push all the parameters into the stack */

  // printf("body code is:\n%s\n", code.c_str());
  string prefix; prefix.clear();
  char func_setting_str[20];
  sprintf(func_setting_str, " [%d] [%d]\n", this->param_num, this->stack_size);
  prefix = this->name + (string)func_setting_str;
  for(int i = 0; i < 12; ++i) {
    char reg_name[10];
    if(s_reg_flag[i]) {
      check(s_reg_memoryAddr[i].length() != 0, "unallocated s_reg");
      sprintf(reg_name, "s%d", i);
      prefix += "store " + (string)reg_name + " " + s_reg_memoryAddr[i] + "\n";
    }
  }
  for(int i = 0; i < this->param_num; ++i) {
    char reg_name[10];
    sprintf(reg_name, "a%d", i);
    char param_name[10];
    sprintf(param_name, "p%d", i);
    prefix += "store " + (string)reg_name + " " + varmap[(string)param_name].memoryAddr + "\n";
  }

  string suffix = "end " + this->name + "\n";

  return prefix + code + suffix;
  /* at the end of the code, we have to pop all used s_registers to stack */
}

void Function::overflow(SentList::iterator it_this, int var_id) {
  SentList::iterator it_tmp = it_this;
  while(it_tmp != sentlist.begin() && it_tmp->varset[var_id]) {
    it_tmp->varset.reset(var_id);
    it_tmp--;
  }
  it_tmp->varset.reset(var_id);
  it_tmp = it_this; it_tmp++;
  while(it_tmp != sentlist.end() && it_tmp->varset[var_id]) {
    it_tmp->varset.reset(var_id);
    it_tmp++;
  }
  string name = id2name[var_id];
  varmap[name].isOverflowed = true;
  if(!varmap[name].isGlobal && !varmap[name].isArray)
    varmap[name].memoryAddr = stackAllocate(1);

}

string Function::stackAllocate(int size) {
  char str[20];
  sprintf(str, "%d", stack_size);
  stack_size += size;
  return (string)str;
}

void Function::loadVar(string var_name, string& load_sent, string& reg_name) {
  Variable var = varmap[var_name];
  load_sent.clear();

  int pid = idParameter(var_name);
  if(var.isOverflowed) {
    reg_name = getTmpReg();
    if(var.isArray)
      load_sent = "loadaddr " + var.memoryAddr + " " + reg_name + "\n";
    else
      load_sent = "load " + var.memoryAddr + " " + reg_name + "\n";
    return;
  }
  else {
    reg_name = getReg(var_name);
    if(var.isLoaded || (!var.isGlobal && !var.isArray && pid == -1)) {
      return;
    }
    else {
      varmap[var_name].isLoaded = true;
      if(var.isArray) { /* is Array, need to load addr */
        load_sent = "loadaddr " + var.memoryAddr + " " + reg_name + "\n";
      }
      else {  /* not parameter variable */
        load_sent = "load " + var.memoryAddr + " " + reg_name + "\n";
      }
      return;
    }
  }
}

void Function::storeVar(string var_name, string& store_sent, string& reg_name) {
  Variable var = varmap[var_name];
  check(!var.isArray, "invalid store for array");

  store_sent.clear();
  if(var.isOverflowed) {
    reg_name = getTmpReg();
    if(var.isGlobal) {
      string array_reg = getTmpReg(2);
      store_sent = "loadaddr " + var.memoryAddr + " " + array_reg + "\n";
      store_sent += array_reg + "[0] = " + reg_name + "\n";
      return;
    }
    else {
      store_sent = "store " + reg_name + " " + var.memoryAddr + "\n";
      return;
    }
  }
  else {
    reg_name = getReg(var_name);
    varmap[var_name].isLoaded = true;
    if(var.isGlobal) {
      string array_reg = getTmpReg();
      store_sent = "loadaddr " + var.memoryAddr + " " + array_reg + "\n";
      store_sent += array_reg + "[0] = " + reg_name + "\n";
    }
    return;
  }
}

string Function::getTmpReg(int n) {
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

string Function::getReg(string name) {
  int reg = varmap[name].reg;
  if(reg == -1) // this variable is not assigned, actually is a dead variable,
                // but as we don't do dead-code-elimination, we have to take it...
    return getTmpReg();
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

void Env::analyze() {
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
    it_func->livenessAnalyze();
    it_func->registerAllocate();
    code += it_func->codeGenerate();
  }

  printf("%s\n", code.c_str());
}
