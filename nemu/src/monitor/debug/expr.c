#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ , HEX , NUM , REG , TK_AND , TK_OR , TK_NEQ , TK_MINUS , TK_DER

  /* TODO: Add more tokens types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal
  {"0[xX][0-9a-fA-F]+",HEX},
  {"0|[1-9][0-9]*",NUM},
  {"\\-",'-'},
  {"\\*",'*'},
  {"\\/",'/'},
  {"\\(",'('},
  {"\\)",')'},
  {"\\$[a-z]+",REG},
  //{"\\$e(([abcd]x)|([sb]p)|([sd]i)|(ip))",REG},
  //{"\\$eip",REG},
  {"&&",TK_AND},
  {"\\|\\|",TK_OR},
  {"!=",TK_NEQ}

};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct tokens {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        tokens[nr_token].type = rules[i].token_type;


        /* TODO: Now a new tokens is recognized with rules[i]. Add codes
         * to record the tokens in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case NUM:
          case REG:
          case HEX:
            if(substr_len>=32) {
              assert(0);
            }
            else {
              for(int j=0;j<substr_len;++j) {
                tokens[nr_token].str[j] = substr_start[j];
              }
              tokens[nr_token++].str[substr_len] = '\0';
            }
            break;

          default: 
            nr_token++;
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  assert(nr_token>0);
  if(tokens[0].type == '-') {
    tokens[0].type = TK_MINUS;
  } 
  else if(tokens[0].type == '*') {
    tokens[0].type = TK_DER;
  }

  for(int j=1;j<nr_token;++j) {
    if(tokens[j].type == '-' && tokens[j-1].type != ')' && ( tokens[j-1].type > REG || tokens[j-1].type < HEX)) 
      tokens[j].type = TK_MINUS;
    else if(tokens[j].type == '*' && tokens[j-1].type != ')' && ( tokens[j-1].type > REG || tokens[j-1].type < HEX))
      tokens[j].type = TK_DER;
  }

  return true;
}


bool parenthesis_right();
uint32_t eval(int p, int q);
int priority(int type);
int find_dominant_operator(int p, int q);
bool check_parentheses(int p, int q);
uint32_t get_num(char str);

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // TODO();
  if(!parenthesis_right()) {
    *success = false;
    return 0;
  }
  else {
    return eval(0,nr_token - 1);
  }

  return 0;
}
bool parenthesis_right() {
  int nums = 0;
  for (int i = 0; i < nr_token; ++i) {
    if (tokens[i].type == '(')
      nums++;
    else if (tokens[i].type == ')')
      nums--;

    if (nums < 0)
      return false;
  }

  return true;
}
uint32_t eval(int p, int q) {
  if(p > q) {
    assert(0);
  }
  else if (p == q){
    if (tokens[p].type == NUM) {
      return atoi(tokens[p].str);
    }
    else if (tokens[p].type == REG) {
      if      (strcmp(tokens[p].str, "$eax") == 0)  return cpu.eax;
      else if (strcmp(tokens[p].str, "$ebx") == 0)  return cpu.ebx;
      else if (strcmp(tokens[p].str, "$ecx") == 0)  return cpu.ecx;
      else if (strcmp(tokens[p].str, "$edx") == 0)  return cpu.edx;
      else if (strcmp(tokens[p].str, "$ebp") == 0)  return cpu.ebp;
      else if (strcmp(tokens[p].str, "$esp") == 0)  return cpu.esp;
      else if (strcmp(tokens[p].str, "$esi") == 0)  return cpu.esi;
      else if (strcmp(tokens[p].str, "$edi") == 0)  return cpu.edi;
      else if (strcmp(tokens[p].str, "$eip") == 0)  return cpu.eip;
      else if (strcmp(tokens[p].str, "$ah") == 0)  return reg_b(R_AH);
      else if (strcmp(tokens[p].str, "$al") == 0)  return reg_b(R_AL);
      else if (strcmp(tokens[p].str, "$bh") == 0)  return reg_b(R_BH);
      else if (strcmp(tokens[p].str, "$bl") == 0)  return reg_b(R_AH);
      else if (strcmp(tokens[p].str, "$ch") == 0)  return reg_b(R_CH);
      else if (strcmp(tokens[p].str, "$cl") == 0)  return reg_b(R_CL);
      else if (strcmp(tokens[p].str, "$dh") == 0)  return reg_b(R_DH);
      else if (strcmp(tokens[p].str, "$dl") == 0)  return reg_b(R_DL);
      else if (strcmp(tokens[p].str, "$ax") == 0)  return reg_w(R_AX);
      else if (strcmp(tokens[p].str, "$bx") == 0)  return reg_w(R_BX);
      else if (strcmp(tokens[p].str, "$cx") == 0)  return reg_w(R_CX);
      else if (strcmp(tokens[p].str, "$dx") == 0)  return reg_w(R_DX);
      else if (strcmp(tokens[p].str, "$sp") == 0)  return reg_w(R_SP);
      else if (strcmp(tokens[p].str, "$bp") == 0)  return reg_w(R_BP);
      else  assert(0);
    }
    else if (tokens[p].type == HEX) {
      int cnt, i, len, sum = 0;
      len = strlen(tokens[p].str);
      cnt = 1;
      for (i = len-1; i >= 0; i--) {
        sum = sum + cnt * get_num(tokens[p].str[i]);
        cnt *= 16;
      }
      return sum;
    }
  }
  else if (check_parentheses(p, q)){
    return eval(p + 1, q - 1);
  }
  else {
    int op = find_dominant_operator(p, q);
    if(op == -1) {
      if(tokens[p].type == TK_MINUS) 
        return -eval(p+1,q);
      else if(tokens[p].type == TK_DER);
        return vaddr_read(eval(p+1,q),4);
      assert(0);
    }
    //printf("op = %d\n", op);
    uint32_t val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);
    //printf("op = %d val1 = %u val2 = %u\n", op, val1, val2);

    switch (tokens[op].type) {
      case '+':
        return val1 + val2;
      case '-':
        return val1 - val2;
      case '*':
        return val1 * val2;
      case '/':
        return val1 / val2;
      case TK_AND:
        return val1 && val2;
      case TK_OR:
        return val1 || val2;
      case TK_EQ:
        return val1 == val2;
      case TK_NEQ:
        return val1 != val2;
      default:
        assert(0);
    }
  }
  return 1;
}
bool check_parentheses(int p, int q) {
  int i, nums = 0;

  for (i = p; i <= q; ++i) {
    if (tokens[i].type == '(') {
      nums++;
    }
    if (tokens[i].type == ')') {
      nums--;
    }
    if(nums == 0 && i < q) {
      return false;
    }
  }

  return true;
}
int find_dominant_operator(int p, int q) {
  int i,nums=0,now_priority=6,now_position=-1,temp;
  for(i=p;i<=q;++i) {
    if(tokens[i].type == '(') {
      nums++;
      continue;
    }
    else if(tokens[i].type == ')') {
      nums--;
      continue;
    }
    else if(nums > 0) 
      continue;
    else if(priority(tokens[i].type)==0)
      continue;
    else if((temp=priority(tokens[i].type))<=now_priority){
      now_position = i;
      now_priority = temp;
    }
  }
  return now_position;
}

int priority(int type) {
  switch (type) {
    case NUM:
    case REG:
    case HEX:
    case TK_MINUS:
    case TK_DER:
      return 0;
    case TK_OR:
      return 1;
    case TK_AND:
      return 2;
    case TK_EQ:
    case TK_NEQ:
      return 3;
    case '+':
    case '-':
      return 4;
    case '*':
    case '/':
      return 5;
    default:
      assert(0);
  }
}

uint32_t get_num(char str)
{
  if (str >= '0' && str <= '9') 
    return str - '0';
  else if (str >= 'a' && str <= 'f') 
    return str - 'a' + 10;
  else if (str >= 'A' && str <= 'F') 
    return str - 'A' + 10;
  return 0;
}
