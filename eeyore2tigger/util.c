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

Function::Function()  {
  stack_size = 0;
  var_num = 0;
  reg_num = 19;
  memset(s_reg_flag, 0, sizeof(s_reg_flag));
  memset(t_reg_flag, 0, sizeof(t_reg_flag));
  memset(a_reg_flag, 0, sizeof(a_reg_flag));

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

  varmap.clear();
  id2name.clear();

  for(int i = 0; i < 12; ++i)
    s_reg_memoryAddr[i].clear();
  for(int i = 0; i < 7; ++i)
    t_reg_memoryAddr[i].clear();
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
        // assume that only assin sentences and con_jump sentences will accur integer
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

  /* count the liveness length for each variable */
  /* under the assumption of "one line for each variable" */
  var_live_length.resize(var_num);
  for(int i = 0; i < var_num; ++i)
    var_live_length[i] = 0;
  for(rit_this = sentlist.rbegin(); rit_this != sentlist.rend(); rit_this++) {
    for(int i = 0; i < var_num; ++i)
      var_live_length[i] += (rit_this->varset[i]) ? 1 : 0;
  }
}

void Function::registerAllocate() {
  SentList::iterator it_this, it_last, it_next;
  for(it_this = sentlist.begin(); it_this != sentlist.end(); it_this++) {
    while(it_this->varset.count() > reg_num) {
      int max_length = 0, max_i = 0;
      for(int i = 0; i < var_num; ++i) {
        if(it_this->varset[i] && max_length < var_live_length[i]) {
          max_length = var_live_length[i];
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
        /* end of DU-Chain, release related register */
        if(it_this->varset[i] && !it_next->varset[i]) {
          if(!var_assigned[i]) {
            int reg_cnt = 0;
            for(; reg_cnt < reg_num && reg_assigned[reg_cnt]; reg_cnt++);
            check(reg_cnt < reg_num, "unchecked overflow");
            varmap[id2name[i]].reg = reg_cnt;
            reg_assigned[reg_cnt] = true;
            var_assigned[i] = true;
          }
          check(var_assigned[i], "release before assign");
          reg_assigned[varmap[id2name[i]].reg] = false;
        }
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
  for(int i = 0; i < this->param_num; ++i) {
    char param_name[10];
    sprintf(param_name, "p%d", i);
    varmap[(string)param_name].isLoaded = false;
    varmap[(string)param_name].memoryAddr = stackAllocate(1);
  }
  string offset;
  SentList::iterator it_param;
  for(it_this = sentlist.begin(); it_this != sentlist.end(); it_this++) {
    switch(it_this->type) {
      case LABEL:
        line = it_this->var1 + "\n";
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
          line = reg1 + " = " + reg2 + "\n";
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
    /* in the front of the code, we have to push all used s_registers to stack,  */
    /* and push all overflowed parameters into the stack */
    /* and push all the parameters into the stack */

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
