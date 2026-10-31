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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include "tab_complete.h"
#include "ops.h"
#include "words.h"

/* Walks ops[] first, then user words; both are live, so new words complete at once. */
static char *generator(const char *text, int state) {
  static int i;
  static size_t len;
  if (!state) { i = 0; len = strlen(text); }
  while (i < n_ops) {
    const char *name = ops[i++].name;
    if (strncmp(name, text, len) == 0) return strdup(name);
  }
  while (i - n_ops < word_count) {
    const char *name = words[i++ - n_ops].name;
    if (strncmp(name, text, len) == 0) return strdup(name);
  }
  return NULL;
}

static char **completion(const char *text, int start, int end) {
  (void)start;
  (void)end;
  rl_attempted_completion_over = 1;   /* never fall back to file names */
  return rl_completion_matches(text, generator);
}

void completion_init(void) {
  /* Only whitespace separates tokens; the default set would split "%chg", ">deg", "||". */
  rl_basic_word_break_characters = " \t\n";
  rl_attempted_completion_function = completion;
}
