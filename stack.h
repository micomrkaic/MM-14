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
#ifndef STACK_H
#define STACK_H

#include "state.h"

static inline int stack_depth(const Stack *s) { return s->top + 1; }

/* Returns RPN_OK, or RPN_ERR on overflow (stack unchanged). */
int push(Stack *s, double v);
/* Caller guarantees depth >= 1 (the dispatcher checks arity). */
double pop(Stack *s);
/* d = 0 is the top (X), 1 is Y, ... Caller guarantees d < depth. */
double peek(const Stack *s, int d);

#endif
