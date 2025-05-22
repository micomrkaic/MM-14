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

// **** Stack structure ****
#define MAX_STACK 1024

typedef struct {
  double data[MAX_STACK];
  int top;
} Stack;

// Stack functions
void initStack(Stack* s);
int isEmpty(Stack* s);
int isFull(Stack* s);
void push(Stack* s, double value);
double pop(Stack* s);


// Copy stack state
void copyStack(Stack* dest, Stack* src);

// Stack operations
void stack_roll(Stack* s);
void stack_nip(Stack* s);
void stack_tuck(Stack* s);
void stack_dup(Stack* s);
void stack_swap(Stack* s);
void stack_drop(Stack* s);
void stack_count(Stack* s);
void stack_sum(Stack* s);
void stack_sum_squares(Stack* s);

#endif // STACK_H
