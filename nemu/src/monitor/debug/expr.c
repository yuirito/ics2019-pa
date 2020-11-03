#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#include <stdlib.h>
uint32_t isa_reg_str2val(const char *s,bool *success);

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */

  /* PA1.2*/
  TK_NUM, TK_PLUS, TK_SUB, TK_MUL, TK_DIV,
  TK_LBR, TK_RBR, TK_HEX, TK_REG, TK_AND, TK_OR, TK_DEREF, TK_NEGTIVE
};

/* PA1.2 */
int check_parentheses(int p, int q);
uint32_t expr(char *e, bool *success);
int get_main_op(int p, int q);
uint32_t eval(int p, int q, bool *success);




static struct rule {
  char *regex;
  int token_type;
  int priority; // add priority
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  /* PA1.2 */
  /* rule,symbole,priority */

  {" +", TK_NOTYPE, 0},    // spaces
  {"\\+", TK_PLUS, 4},         // plus
  {"==", TK_EQ, 3},         // equal
  {"-", TK_SUB, 4},		// substract
  {"\\*", TK_MUL, 5},		// multiply or derefrence
  {"/", TK_DIV, 5},		// divide
  {"0[Xx][0-9a-fA-F]+", TK_HEX, 0},	// hex (must before the TK_NUM)
  {"[0-9]+", TK_NUM, 0},	// number(dec)
  {"\\(", TK_LBR, 7},		// left bracket
  {"\\)", TK_RBR, 7},		// right bracket
  {"\\$[a-zA-Z]{2,3}", TK_REG, 0},   // register
  {"&&", TK_AND, 2},		// and
  {"\\|\\|", TK_OR, 1}		// or
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

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
  int priority; //add priority
} Token;

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

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

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        /* PA1.2 */
        if(substr_len>32){
            Log("The length of the substring is too long");
            assert(0);
        }

        switch (rules[i].token_type) {
            case TK_NOTYPE: break;
            case TK_NUM:
            case TK_HEX:
            case TK_REG:
                strncpy(tokens[nr_token].str, substr_start, substr_len);
				        tokens[nr_token].str[substr_len] = '\0';
          default: 
                tokens[nr_token].type = rules[i].token_type;
                tokens[nr_token].priority = rules[i].priority;
				        nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  /* PA1.2*/
  for(int i=0; i<nr_token; i++){
    if(tokens[i].type == TK_MUL && (i==0 || (tokens[i-1].priority!=0 && tokens[i-1].type!=TK_RBR))){
       tokens[i].type = TK_DEREF;
       tokens[i].priority = 6;
    }
    else if (tokens[i].type == TK_SUB && (i==0 || (tokens[i-1].priority!=0 && tokens[i-1].type!=TK_RBR))){
       tokens[i].type = TK_NEGTIVE;
       tokens[i].priority = 6;
    }
  }
  *success = true;
  uint32_t result = eval(0, nr_token-1, success);
  return result;
}

/* PA1.2 */

/*  return 1  if the parentheses is a valid expression with left and right bracket.
 *  return 0  if the parentheses is a valid expression but without left and right bracket.
 *  return -1 if the parenteses isn't a valid expression. 
 */

int check_parentheses(int p, int q)
{
	bool LR = false;
	if(tokens[p].type == TK_LBR && tokens[q].type ==TK_RBR) LR = true;
	int nump = 0, i;
	for(i = p; i <= q; i++){
		if(tokens[i].type == TK_LBR) nump++;
		else if(tokens[i].type == TK_RBR) nump--;
		
    if(nump < 0) return -1;
	}
	
  if(nump != 0) return -1;
  if(!LR) return 0;  // without left and right bracket

	/* check if the left and right brackets are the part of the expression like (exp)+(exp) */
	for(i = p + 1; i <= q - 1; ++ i){
		if(tokens[i].type == TK_LBR) nump++;
		else if(tokens[i].type == TK_RBR) nump--;
		if(nump < 0) return 0;
	}
	return 1;	
}


/* return -1 if there is no main operator */
int get_main_op(int p, int q)
{
	int inBracket = 0, i, pos = -1, priority=7;
	for(i = p; i <= q; i++) {
		int type = tokens[i].type;
		if( !inBracket && tokens[i].priority>0&&tokens[i].priority<7){
			if(tokens[i].priority<=priority ){
        pos = i;
        priority = tokens[i].priority;
      }  
		}
		else if(type == TK_LBR ) inBracket ++ ;
		else if(type == TK_RBR ) inBracket -- ;
	}
	return pos;
}

uint32_t eval(int p, int q, bool *success)
{
	if(p > q) {
		*success = false;
    Log("p>q");
    return -1;
	}else if(p == q){
		uint32_t val = 0;
		int type = tokens[p].type;
		if(type == TK_NUM || type == TK_HEX) {
			return strtoul(tokens[p].str, NULL, 0);
		}
		else if(type == TK_REG) {
			val = isa_reg_str2val(tokens[p].str + 1, success);
			if(*success) return val;
			printf("Unknown register: %s\n", tokens[p].str);
			return 0; 
		}
    *success = false;
		return -1;
	}
	int ret = check_parentheses(p, q);
	if(ret == -1) {
    *success = false;
		Log("check_parentheses return -1");
    
    return -1;
	}
	
	if(ret == 1) {
		return eval(p + 1, q - 1, success);
	}
	
	int pos = get_main_op(p, q);
	if(pos == -1){
    *success = false;
		return -1;
	}
	uint32_t val1 = 0, val2 = 0, val = 0;
	if(tokens[pos].type != TK_DEREF && tokens[pos].type != TK_NEGTIVE)
		val1 = eval(p, pos - 1, success);
	if(*success == false) 
    return 0;

	val2 = eval(pos + 1, q, success);
	if(*success == false) 
    return 0;

	switch(tokens[pos].type){
		case TK_PLUS:
			val = val1 + val2;
			break;
		case TK_SUB:
			val = val1 - val2;
			break;
		case TK_MUL:
			val = val1 * val2;
			break;
		case TK_DIV:
			if(val2 == 0) {
				printf("Divide 0 error at [%d, %d]", p, q);
				return *success = false;
			}
			val = val1 / val2;
			break;
		case TK_AND:
			val = val1 && val2;
			break;
		case TK_OR:
			val = val1 || val2;
			break;
		case TK_EQ:
			val = val1 == val2;
			break;
		case TK_DEREF:
			val = vaddr_read(val2, 4);
			break;
    case TK_NEGTIVE:
      val = -val2;
      break;	
		default:
			printf("Unknown token type %d: %d %d\n",pos, tokens[pos].type,TK_PLUS);
			return *success = false;
	}

	return val;
}