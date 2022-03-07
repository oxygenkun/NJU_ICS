#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>  // bool support
#include <stdio.h>    // sscanf support
#include <inttypes.h> // u_int32_t sscanf macro support
#include <ctype.h>
#include <memory/vaddr.h>
enum {
  // not start from small number (0) as it will be the same in initial constructon
  TK_NOTYPE = 256, 
  TK_EQ,
  TK_NEQ,
  TK_AND,
  TK_PLUS,
  TK_MINUS,
  TK_MULT,
  TK_DIV,
  TK_LP,
  TK_RP,
  TK_INT,
  TK_HEX,
  TK_REG,
  TK_DEREF,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

    /* DONE: Add more rules.
     * Pay attention to the precedence level of different rules.
     */
    {" +", TK_NOTYPE},           // spaces
    {"0x[[:xdigit:]]+", TK_HEX}, // hex number
    {"\\$[[:alnum:]_]+", TK_REG},    // register var
    {"[[:digit:]]+", TK_INT},    // integer
    {"==", TK_EQ},               // equal
    {"!=", TK_NEQ},
    {"&&", TK_AND},
    {"\\+", TK_PLUS},            // plus
    {"-", TK_MINUS},             // minus
    {"\\*", TK_MULT},            // multiply
    {"/", TK_DIV},               // divide
    {"\\(", TK_LP},              // left parenthese
    {"\\)", TK_RP},              // right parentheseq
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

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

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65535] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  regmatch_t pmatch;

  while (e[position] != '\0') {
    int i;
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        int tk_type = rules[i].token_type;
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
           i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* DONE: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        // additional check
        if((tk_type == TK_INT || tk_type == TK_HEX)
              && substr_len >= 32){
          printf("EXPR has a large number out of bound at position %d\n%s\n%*.s^\n", position, e, position, "");
          return false;
        }
        tokens[nr_token].type = tk_type;
        strncpy(tokens[nr_token].str, substr_start, substr_len);
        nr_token++;

        break; // break for-loop
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static bool check_parentheses(int p, int q) {
  if (tokens[p].type != TK_LP || tokens[q].type != TK_RP){
    return false;
  }

  for (int i = p, nr_parentheses = 0; i <= q; ++i) {
    if (tokens[i].type == TK_LP) {
      ++nr_parentheses;
    } else if (tokens[i].type == TK_RP) {
      --nr_parentheses;
      if (nr_parentheses < 0) {break;}
      else if (nr_parentheses == 0) {
        // if right equals left, but i is not at the end, reutrn false.
        // if right equals left and i is at the end, reutrn true.
          return (i==q);
      } // else continue
    }
  } // right less than left, return false.
  return false;
}

static int find_main_op(int p, int q) {
  int op = -1; // the position of 主运算符 in the token expression;
  int in_parentheses = 0;
  // find main op (escape op in parentheses)
  for (int i = p; i <= q; ++i) {
    switch (tokens[i].type) {
    case TK_LP:
      ++in_parentheses;
      break;
    case TK_RP:
      --in_parentheses;
      break;
    case TK_PLUS:
    case TK_MINUS:
      if (in_parentheses == 0) {
        op = i;
      }
      break;
    case TK_MULT:
    case TK_DIV:
      if (in_parentheses == 0 &&
          (op == -1 || tokens[op].type == TK_MULT || tokens[op].type == TK_DIV)) {
        // 1. not in the parentheses
        // 2. plus or minus not exist or pre op type is mult or div
        op = i;
      }
      break;
    default:
      break;
    }
  }
  return op;
}

// eval EXPR
static u_int32_t eval(int p, int q, bool* err){
  if (*err) return 0;
  if (p > q) {
    *err = true;
    return 0;
  } else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    u_int32_t ret;
    switch (tokens[p].type) {
    case TK_INT:
      if (sscanf(tokens[p].str, "%" SCNu32, &ret) != 1) {
        printf("parse failed: %s as INT\n", tokens[p].str);
        *err = true;
        return 0;
      }
      return ret;
    case TK_HEX:
      for (int i = 0; tokens[p].str[i] != '\0'; ++i) {
        tokens[p].str[i] = tolower(tokens[p].str[i]);
      }
      if (sscanf(tokens[p].str, "%x", &ret) != 1) {
        printf("parse failed: %s as HEX\n", tokens[p].str);
        *err = true;
        return 0;
      }
      return ret;
    case TK_REG:
      return isa_reg_str2val(tokens[p].str + 1, err);
    default:
      printf("bad expr: '%s'\n", tokens[p].str);
      *err = true;
      return 0;
    }
  } else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, err);
  } else if (tokens[p].type == TK_DEREF) {
    u_int32_t ret = eval(p+1, q, err);
    if(*err){
      printf("derefference failed at position: %d\n", p+1);
      return 0;
    }
    return vaddr_read(ret, 4);
  }

  // find main op
  int op = find_main_op(p, q);
  if(op==-1){
    printf("Bad expr!\n");
    *err=true;
    return 0;
  }

  // compute
  u_int32_t val1 = eval(p, op - 1, err);
  u_int32_t val2 = eval(op + 1, q, err);
  switch (tokens[op].type) {
    case TK_PLUS: return val1 + val2;
    case TK_MINUS: return val1 - val2;
    case TK_MULT: return val1 * val2;
    case TK_DIV: 
      if (val2==0){
        printf("Division by zero\n");
        *err = true;
        return 0;
      }
      return val1 / val2;
    default: assert(0);
  }
}

static bool dereference_pre_type(int type){
  return (type != TK_INT || type != TK_HEX || type != TK_REG || type != TK_RP);
} 

static void empty_process(){
  // emptify tokens
  for(int i=0; i<nr_token; ++i){
    memset(tokens[i].str, '\0', 32);
    tokens[i].type=0;
  }
  nr_token=0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e) || nr_token <= 0) {
    *success = false;
    empty_process();
    return 0;
  }

  // dereference
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == TK_MULT &&
        (i == 0 || dereference_pre_type(tokens[i - 1].type))) {
      tokens[i].type = TK_DEREF;
    }
  }

  /* Done: Insert codes to evaluate the expression. */
  bool err = false; // true if EXPR is illegal
  u_int32_t ret = eval(0,nr_token-1, &err);
  *success = !err;

  empty_process();
  return ret;
}
