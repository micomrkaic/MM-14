/*
 * This file is part of MM-14.
 *
 * MM-14 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MM-14 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MM-14 If not, see <https://www.gnu.org/licenses/>.
 */
/* Mico's simple RPN Calculator -- MM-14

   Still to do as of May 20, 2025
   
   for MM-14
   . nicer and more organized help screen
   . documentation -- README.md file etc.

   for MM-15
   . run, load and save programs
   . complex number support (for mm-15)
   . matrices
   . checks if any terms words; definition are undefined

   for MM-48
   0. recursive descent parser
   1. simple plotting inside the terminal??
   2. symbolic stuff
   3. strings
   4. lists
   5. units and constants
   6. printing and prompting commands
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "globals.h"
#include "stack.h"
#include "my_math.h"
#include "stack_printing.h"
#include "my_statistics.h"
#include "words.h"
#include "help_functions.h"
#include "registers.h"
#include "util_func.h"
#include "tab_complete.h"
#include "tokenizer.h"
#include "interpreter.h"

int main() {
  Stack stack;
  Stack old_stack;
  char* input = NULL;

  splash_screen();
  rl_attempted_completion_function = rpn_completion;

  initStack(&stack);
  copyStack(&old_stack, &stack);

  load_words_from_file();
  intg_function = 5;
    
  // Main REPL loop
  while ((input = readline("> "))) {
    if (!input || strlen(input) == 0) {
      printStack(&stack, precision,"Stack");
      free(input);
      continue;
    }
    if (*input) add_history(input);

    evaluate_input(input, &stack, &old_stack);
    printStack(&stack, precision,"Stack");
    free(input);
    if (terminate_program) goto nice_exit;
  }

 nice_exit:
  clear_history();  // frees history list memory
  printf("Goodbye!\n");
  return 0;
}
