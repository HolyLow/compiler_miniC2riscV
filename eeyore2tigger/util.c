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

/*
bool Sentence::use(string var) {
  switch(type) {
    case LABEL:
    case CALL:
    case CALL_ASSIGN:
      return false;
    case JUMP:
    case CON_JUMP:
    case PARAM:
    case RETURN:
    case ASSIGN:
      return (var == var1);
    case OP_1:
      return (var == var2);
    case OP_2:
    case LOAD:
      return (var == var2) || (var == var3);
    case STORE:
      return (var == var1) || (var == var2) || (var == var3);
    default:
      printf("undefined SentenceType\n");
      exit(1);
  }
}

bool Sentence::define(string var) {
  switch(type) {
    case LABEL:
    case JUMP:
    case CON_JUMP:
    case PARAM:
    case RETURN:
    case CALL:
    case STORE:
      return false;
    case CALL_ASSIGN:
    case OP_1:
    case OP_2:
    case ASSIGN:
    case LOAD:
      return (var == var1);
    default:
      printf("undefined SentenceType\n");
      exit(1);
  }
}
*/

Function::Function()  {
  stack_size = 0;
  var_num = 0;
  reg_num = 19;
  memset(s_reg_flag, 0, sizeof(s_reg_flag));
  memset(t_reg_flag, 0, sizeof(t_reg_flag));

  varmap.clear();
  id2name.clear();
}

void Function::clear() {
  stack_size = 0;
  var_num = 0;
  reg_num = 19;
  memset(s_reg_flag, 0, sizeof(s_reg_flag));
  memset(t_reg_flag, 0, sizeof(t_reg_flag));

  varmap.clear();
  id2name.clear();
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
