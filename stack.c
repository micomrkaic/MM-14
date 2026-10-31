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
#include "stack.h"

int push(Stack *s, double v) {
  if (s->top >= MAX_STACK - 1) return rpn_err("Stack overflow (max %d)", MAX_STACK);
  s->data[++s->top] = v;
  return RPN_OK;
}

double pop(Stack *s) {
  return (s->top >= 0) ? s->data[s->top--] : 0.0;
}

double peek(const Stack *s, int d) {
  return (d >= 0 && d <= s->top) ? s->data[s->top - d] : 0.0;
}
