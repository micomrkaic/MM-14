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
#define _POSIX_C_SOURCE 200809L
#include "tab_complete.h"
#include <stdlib.h>

// Word list for TAB completion
const char *rpn_words[] = {
  "dup", "drop", "swap", "nip", "tuck", "roll",
  "==", "<=", ">=", "!=", "<", ">", "&&", "||", "!",
  "+", "-", "*", "/", "^", "pow", "abs", "inv",
  "sin", "cos", "tan", "asin", "acos", "atan",
  "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
  "log", "ln", "chs", "exp", "%", "%chg",
  "npdf", "ncdf", "nqnt", "frac", "int","integrate","fzero",
  "s+", "s-", "clstat", "getstats",
  "lstx", "lsty", "stacksize", "stacksum", "stacksumsq",
  "clst", "u", "p_prec", "q", "help", "sss", "spf", "swapterm",
  "int_prec","f0_prec","word_sel","internals",
  "sto", "rcl", "listregs", "clrg", "loadregs", "saveregs","regidx",
  "sto+", "sto-", "sto*","sto/",
  "listwords", "clearwords", "delword", "savewords", "loadwords",
  NULL
};

static char *command_generator(const char *text, int state) {
  static int list_index, len;
  const char *name;
  static char match[64];

  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  while ((name = rpn_words[list_index++])) {
    if (strncmp(name, text, len) == 0) {
      snprintf(match, sizeof(match), "%s", name);
      return strdup(match);
    }
  }
  return NULL;
}

char **rpn_completion(const char *text, int start, int end) {
  (void)start;
  (void)end;
  return rl_completion_matches(text, command_generator);
}
