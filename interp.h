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
#ifndef INTERP_H
#define INTERP_H

#include <stdbool.h>
#include "state.h"

/* Line-level interpreter. State that spans lines: an open definition
   (": name ... ;" may cover several lines) and an open "( ... )" comment. */
typedef struct {
  enum { IN_NORMAL, IN_DEF_NAME, IN_DEF_BODY } mode;
  char   name[MAX_WORD_NAME];
  char **toks;
  int    ntok, cap;
  bool   in_comment;
} Interp;

void interp_init(Interp *in);
void interp_free(Interp *in);

/* Runs one line. Each top-level token is atomic: on error the state is
   restored to just before that token and the rest of the line is skipped.
   Returns RPN_OK or RPN_ERR. */
int  interp_line(Interp *in, State *s, char *line);

/* True while a definition or comment is open (for the continuation prompt). */
bool interp_pending(const Interp *in);

/* Call at end of input: reports an unterminated definition. */
int  interp_finish(Interp *in);

#endif
