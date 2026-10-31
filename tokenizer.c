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
#include <string.h>
#include "tokenizer.h"

Tokenizer make_tokenizer(char *input, const char *delim) {
  Tokenizer t = { input, input, delim };
  return t;
}

char *next_token(Tokenizer *t) {
  if (!t->current) return NULL;

  t->current += strspn(t->current, t->delim);
  if (*t->current == '\0') return NULL;

  char *start = t->current;
  t->current = strpbrk(start, t->delim);

  if (t->current) {
    *t->current = '\0';
    t->current++;
  }
  return start;
}
