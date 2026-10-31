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
#ifndef OPS_H
#define OPS_H

#include "state.h"

typedef int (*OpFn)(State *s);

/* One row per command. The single source of truth for dispatch, arity
   checking, help, tab completion and COMMANDS.md.
   fn == NULL marks syntax handled by the interpreter/compiler
   (":", ";", "see", "u", comments, control-flow words). */
typedef struct {
  const char *name;
  int         nargs;     /* minimum stack depth required */
  OpFn        fn;
  const char *cat;
  const char *args;      /* stack picture, e.g. "y x -- y+x" */
  const char *help;
} Op;

extern const Op ops[];
extern const int n_ops;

int find_op(const char *name);          /* index or -1 */
int call_op(State *s, int idx);         /* arity check + call */

#endif
