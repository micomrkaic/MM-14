#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "help_functions.h"
#include "util_func.h"
#include "my_statistics.h"
#include "my_math.h"
#include "words.h"
#include "registers.h"
#include "globals.h"
#include "interpreter.h"

// Global loop index (supporting single nesting for now)
int loop_index = 0;

const char *unary_ops[] = {
  "sin", "cos", "tan","asin", "acos", "atan", "sinh", "cosh", "tanh","asinh", "acosh", "atanh",
  "log", "ln", "chs", "exp", "inv", "abs", "!", "npdf", "ncdf", "nqnt","lstx", "lsty","frac", "int",
  NULL
};

const char *binary_ops[] = {
  "+", "-", "*", "/", "^", "pow", "==", ">=", ">", "<", "<=", "!=", "&&", "||", "%", "%chg", "s+", "s-","integrate","fzero",
  NULL
};

bool is_unary_operator(const char *token) {
  for (int i = 0; unary_ops[i] != NULL; i++) {
    if (strcmp(token, unary_ops[i]) == 0)
      return true;
  }
  return false;
}

bool is_binary_operator(const char *token) {
  for (int i = 0; binary_ops[i] != NULL; i++) {
    if (strcmp(token, binary_ops[i]) == 0)
      return true;
  }
  return false;
}

// Perform binary operation
double binaryOp(double a, double b, const char* op) {
  last_y = a;
  last_x = b;
  
  if (op[0] == '+') return a + b;
  if (op[0] == '-') return a - b;
  if (op[0] == '*') return a * b;
  if (op[0] == '/') return safe_div(a,b);
  if (strcmp(op, "^") == 0 || strcmp(op, "pow") == 0) return pow(a, b);
  if (strcmp(op, "==") == 0) return (a==b);
  if (strcmp(op, "<=") == 0) return (a<=b);
  if (strcmp(op, "<") == 0) return (a<b);
  if (strcmp(op, ">") == 0) return (a>b);
  if (strcmp(op, ">=") == 0) return (a>=b);
  if (strcmp(op, "!=") == 0) return (a!=b);
  if (strcmp(op, "&&") == 0) return (a && b);
  if (strcmp(op, "||") == 0) return (a || b);
  if (strcmp(op, "%") == 0) return (a/b*100.0);
  if (strcmp(op, "%chg") == 0) return ((b-a)/a*100.0);
  if (strcmp(op, "s+") == 0) return sigma_plus(a,b);
  if (strcmp(op, "s-") == 0) return sigma_minus(a,b); 
  if (strcmp(op, "integrate") == 0) return integrate(a,b);
  if (strcmp(op, "fzero") == 0) return find_zero(a,b);
  printf("Invalid operator!\n");
  return 0;
}

// Perform unary operation
double unaryOp(double a, const char* op) {
  last_x = a;  // Save the X register

  if (strcmp(op, "frac") == 0) return safe_frac(a);
  if (strcmp(op, "int") == 0) return safe_int(a);
  if (strcmp(op, "nqnt") == 0) return inverse_normal_cdf(a,1.0e-8);
  if (strcmp(op, "npdf") == 0) return normal_pdf(a);
  if (strcmp(op, "ncdf") == 0) return normal_cdf(a);
  if (strcmp(op, "abs") == 0) return fabs(a);
  if (strcmp(op, "!") == 0) return !(a);
  if (strcmp(op, "abs") == 0) return fabs(a);
  if (strcmp(op, "sin") == 0) return sin(a);
  if (strcmp(op, "cos") == 0) return cos(a);
  if (strcmp(op, "tan") == 0) return tan(a);
  if (strcmp(op, "asin") == 0) return asin(a);
  if (strcmp(op, "acos") == 0) return acos(a);
  if (strcmp(op, "atan") == 0) return atan(a);
  if (strcmp(op, "sinh") == 0) return sinh(a);
  if (strcmp(op, "cosh") == 0) return cosh(a);
  if (strcmp(op, "tanh") == 0) return tanh(a);
  if (strcmp(op, "asinh") == 0) return asinh(a);
  if (strcmp(op, "acosh") == 0) return acosh(a);
  if (strcmp(op, "atanh") == 0) return atanh(a);
  if (strcmp(op, "log") == 0) return safe_log10(a);
  if (strcmp(op, "ln") == 0) return safe_ln(a);
  if (strcmp(op, "inv") == 0) return safe_div(1.0,a);
  if (strcmp(op, "chs") == 0) return -a;
  if (strcmp(op, "exp") == 0) return exp(a);
  printf("Invalid function!\n");
  return 0;
}

void process_token(char* token, Stack* stack, Stack* old_stack, Tokenizer* t) {
  //  printf("token inside process_token %s\n",token);

  if (strcmp(token, "q") == 0)          { terminate_program = true; return; } // Quit
  if (strcmp(token, "u") == 0)          { copyStack(stack, old_stack); return; } // Undo

  copyStack(old_stack, stack); // Save the stack before processign the next token
  if (isNumber(token))                  { push(stack, atof(token)); return; }  // Push a single number
  if (strcmp(token, "clst") == 0)       { initStack(stack); return; } // Clear stack
  if (strcmp(token, "stacksize") == 0)  { stack_count(stack); return; } // Number of stack elements
  if (strcmp(token, "stacksum") == 0)   { stack_sum(stack); return; } // Sum of stack elements
  if (strcmp(token, "stacksumsq") == 0) { stack_sum_squares(stack); return; } // Clear stack
  if (strcmp(token, "sss") == 0)        { show_stat_regs = !show_stat_regs; return; } // Show statistics
  if (strcmp(token, "swapterm") == 0)   { small_terminal = !small_terminal; return;} // Small<->Big terminal output
  if (strcmp(token, "delword") == 0)    { if (!stack_underflow) delete_word_by_index((int)pop(stack)); return; }
  if (strcmp(token, "savewords") == 0)  { save_words_to_file(); return; }
  if (strcmp(token, "loadwords") == 0)  { load_words_from_file(); return; }
  if (strcmp(token, "spf") == 0)        { fixed_point_format = !fixed_point_format; return; }
  if (strcmp(token, "clstat") == 0)     { sigma_clear(); return; }
  if (strcmp(token, "getstats") == 0)   { sigma_evaluate(); return; }
  if (strcmp(token, "listregs") == 0)   { list_registers(); return; }
  if (strcmp(token, "listwords") == 0)  { list_words(); return;  }
  if (strcmp(token, "clrg") == 0)       { clear_registers(); return; }
  if (strcmp(token, "saveregs") == 0)   { save_registers_to_file(); return; }
  if (strcmp(token, "loadregs") == 0)   { load_registers_from_file(); return; }
  if (strcmp(token, "clearwords") == 0) { clear_words(); return;  }
  if (strcmp(token, "help") == 0)       { help_screen(); return; }
  if (strcmp(token, "pi") == 0)         { push(stack, M_PI); return; }
  if (strcmp(token, "lstx") == 0)       { push(stack, last_x); return; }
  if (strcmp(token, "lsty") == 0)       { push(stack, last_y); return; }
  if (strcmp(token, "sto") == 0)        { top_to_register(stack); return; }
  if (strcmp(token, "sto+") == 0)       { register_plus_top(stack); return; }
  if (strcmp(token, "sto-") == 0)       { register_minus_top(stack); return; }
  if (strcmp(token, "sto*") == 0)       { register_times_top(stack); return; }
  if (strcmp(token, "sto/") == 0)       { register_divided_by_top(stack); return; }
  if (strcmp(token, "rcl") == 0)        { register_to_top(stack); return;}
  if (strcmp(token, "internals") == 0)  { print_internals(); return;}
  if (strcmp(token, "p_prec") == 0)     { set_disp_precision(stack); return;}
  if (strcmp(token, "int_prec") == 0)   { set_int_precision(stack); return;}
  if (strcmp(token, "f0_prec") == 0)    { set_f0_precision(stack); return;}
  if (strcmp(token, "word_sel") == 0)   { word_select(stack); return;}
  if (strcmp(token, "regidx") == 0)     { set_reg_index(stack); return;}

  if (strcmp(token, ".") == 0) {
    if (isEmpty(stack)) {
      printf("Stack is empty\n");
      return;
    } else {
      printf("%.*f\n", precision, pop(stack));
    }
    return;
  }

  if (strcmp(token, ";") == 0) {
    return; // Do nothing if ; shows up outside a definition
  }
 
  // Binary functions
  if (is_binary_operator(token))
    {
      if (stack->top < 1) {
	printf("Need at least 2 numbers on stack!\n");
      } else {
	double b = pop(stack);
	double a = pop(stack);
	double result = binaryOp(a, b, token);
	push(stack, result);
      }
    }
  else if (is_unary_operator(token))
    {
      if (isEmpty(stack)) {
	printf("Need at least 1 number on stack!\n");
      } else {
	double a = pop(stack);
	double result = unaryOp(a, token);
	push(stack, result);
      }
    }
  
  else if (strcmp(token, "dup") == 0) stack_dup(stack);
  else if (strcmp(token, "swap") == 0) stack_swap(stack);
  else if (strcmp(token, "drop") == 0) stack_drop(stack);
  else if (strcmp(token, "nip") == 0) stack_nip(stack);
  else if (strcmp(token, "tuck") == 0) stack_tuck(stack);
  else if (strcmp(token, "roll") == 0) stack_roll(stack);

  else if (strcmp(token, "DO") == 0) {
    char *loop_body = malloc(MAX_WORD_BODY);
    
    if (!loop_body) {
      fprintf(stderr, "Memory allocation failed in DO\n");
      return;
    }
    loop_body[0] = '\0';
    int nesting = 1;
    char *loop_token;

    while ((loop_token = next_token(t))) {
      if (strcmp(loop_token, "DO") == 0) nesting++;
      else if (strcmp(loop_token, "LOOP") == 0) {
	nesting--;
	if (nesting == 0) break;
      }
      strncat(loop_body, loop_token, MAX_WORD_BODY - strlen(loop_body) - 2);
      strcat(loop_body, " ");
    }
    if (nesting != 0) {
      fprintf(stderr, "Unmatched DO ... LOOP block\n");
      free(loop_body);
      return;
    }
    if (stack->top < 1) {
      fprintf(stderr, "Need limit and start on stack for DO\n");
      free(loop_body);
      return;
    }

    int start = (int)pop(stack);
    int limit = (int)pop(stack);

    for (int i = start; i < limit; i++) {
      loop_index = i;
      char *body_copy = strdup(loop_body);  // deep copy
      if (!body_copy) {
        fprintf(stderr, "Memory allocation failed for loop body\n");
        break;
      }
      Tokenizer body_tok = make_tokenizer(body_copy, " ");
      //      printf("Loop body=%s\n",loop_body);
      char *subtok;
      while ((subtok = next_token(&body_tok))) {
	//	printf("token inside loop body=%s\n",subtok);
	process_token(subtok, stack, old_stack, &body_tok); // recursively process
      }
      free(body_copy);
    }
    return;
  }
  else if (strcmp(token, "I") == 0) {
    push(stack, loop_index);
    return;
  }
 
  // User-defined word?
  else if (strcmp(token, "IF") == 0) {
    char *if_block = malloc(MAX_WORD_BODY);
    if (!if_block) {
      fprintf(stderr, "Memory allocation failed in IF\n");
      return;
    }
    if_block[0] = '\0';

    int nesting = 1;
    char *token_buf = NULL;
    char *else_part = NULL;

    while ((token_buf = next_token(t))) {
      if (strcmp(token_buf, "IF") == 0) {
	nesting++;
      } else if (strcmp(token_buf, "THEN") == 0) {
	nesting--;
	if (nesting == 0) break;
      }

      if (nesting == 1 && strcmp(token_buf, "ELSE") == 0) {
	else_part = strdup(if_block);
	if_block[0] = '\0';
	continue;
      }

      strncat(if_block, token_buf, MAX_WORD_BODY - strlen(if_block) - 2);
      strcat(if_block, " ");
    }

    if (nesting != 0) {
      fprintf(stderr, "Unmatched IF ... THEN block\n");
      free(if_block);
      if (else_part) free(else_part);
      return;
    }

    if (isEmpty(stack)) {
      fprintf(stderr, "Need a value on the stack for IF\n");
      free(if_block);
      if (else_part) free(else_part);
      return;
    }

    double condition = pop(stack);
    char *chosen_block = (condition != 0.0) ? (else_part ? else_part : if_block) : if_block;

    Tokenizer inner = make_tokenizer(chosen_block, " ");
    char *subtok;
    while ((subtok = next_token(&inner))) {
      process_token(subtok, stack, old_stack, &inner);
    }

    free(if_block);
    if (else_part) free(else_part);
    return;
  }

  else {
    UserWord* w = find_word(token);
    if (w) {
      char* body = strdup(w->body);
      Tokenizer sub_t = make_tokenizer(body, " ");
      char* sub;

      while ((sub = next_token(&sub_t)) != NULL) {
	process_token(sub, stack, old_stack, &sub_t);
      }

      free(body);
    } else {
      printf("Invalid input after user defined word : %s\n", token);
    }
  }
}

double evaluate_input(char *input, Stack *stack, Stack *old_stack) {
  Tokenizer t = make_tokenizer(input, " ");
  char *token;

  while ((token = next_token(&t))) {
    if (strcmp(token, ":") == 0) { // Begin: definition of a new word
      char *name = next_token(&t);
      if (!name || word_count >= MAX_WORDS) {
	printf("Invalid or too many word definitions.\n");
      } else {
	UserWord *w = &words[word_count++];
	strncpy(w->name, name, MAX_WORD_NAME);
	w->body[0] = '\0';
	  
	char *def_token;
	while ((def_token = next_token(&t))) {
	  if (strcmp(def_token, ";") == 0) break;
	  strncat(w->body, def_token, MAX_WORD_BODY - strlen(w->body) - 2);
	  strcat(w->body, " ");
	}	  
	printf("Defined word: %s => %s\n", w->name, w->body);
      } continue; // End: definition of a new word
    } else {
      process_token(token, stack, old_stack, &t);
    }
  }
  return stack->data[stack->top];
}

