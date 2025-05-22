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
#include "globals.h"
#include "stack.h"

// Stack functions
void initStack(Stack* s) {
  s->top = -1;
}

int isEmpty(Stack* s) {
  return s->top == -1;
}

int isFull(Stack* s) {
  return s->top == MAX_STACK - 1;
}

void push(Stack* s, double value) {
  if (!isFull(s)) {
    s->data[++s->top] = value;
  } else {
    printf("Stack overflow!\n");
  }
}

double pop(Stack* s) {
  if (!isEmpty(s)) {
    stack_underflow = false;
    return s->data[s->top--];
  } else {
    stack_underflow = true;
    printf("Stack underflow!\n");
    return 0.0;
  }
}

// Copy stack state
void copyStack(Stack* dest, Stack* src) {
  dest->top = src->top;
  for (int i = 0; i <= src->top; i++) {
    dest->data[i] = src->data[i];
  }
}

// Stack operations
void stack_roll(Stack* s) {
  if (s->top < 0) {
    printf("Error: Stack underflow on roll (no roll index)\n");
    return;
  }

  int n = (int)pop(s);  // roll index
  if (n < 0 || n > s->top) {
    printf("Error: Invalid roll index %d (stack has %d items)\n", n, s->top + 1);
    return;
  }

  if (n == 0) return;  // rolling top to top, do nothing

  double value = s->data[s->top - n];
  for (int i = s->top - n; i < s->top; i++) {
    s->data[i] = s->data[i + 1];
  }
  s->data[s->top] = value;
}

void stack_nip(Stack* s) {
  if (s->top >= 1) {
    s->data[s->top - 1] = s->data[s->top]; // overwrite second-to-top with top
    s->top--;                              // pop top
  } else {
    printf("Error: Need at least two elements for nip\n");
  }
}

void stack_tuck(Stack* s) {
  if (s->top >= 1 && s->top < MAX_STACK - 1) {
    double top_val = s->data[s->top];
    s->data[s->top + 1] = s->data[s->top];         // push top again
    s->data[s->top] = s->data[s->top - 1];         // shift second-to-top down
    s->data[s->top - 1] = top_val;                 // insert top under it
    s->top++;
  } else {
    printf("Error: Need at least two elements (and room) for tuck\n");
  }
}

void stack_dup(Stack* s) {
  if (s->top >= 0 && s->top < MAX_STACK - 1) {
    s->data[s->top + 1] = s->data[s->top];
    s->top++;
  } else {
    printf("Error: Cannot dup (stack empty or full)\n");
  }
}

void stack_swap(Stack* s) {
  if (s->top >= 1) {
    double a = s->data[s->top];
    double b = s->data[s->top - 1];
    s->data[s->top] = b;
    s->data[s->top - 1] = a;
  } else {
    printf("Error: Need at least two values to swap\n");
  }
}

void stack_drop(Stack* s) {
  if (s->top >= 0) {
    s->top--;
  } else {
    printf("Error: Stack underflow on drop\n");
  }
}

// Push the number of elements on the stack
void stack_count(Stack* s) {
  if (!isFull(s)) {
    push(s, s->top + 1);
  } else {
    printf("Error: Cannot count, stack is full\n");
  }
}

// Push the sum of all elements on the stack
void stack_sum(Stack* s) {
  if (!isFull(s)) {
    double total = 0.0;
    for (int i = 0; i <= s->top; i++) {
      total += s->data[i];
    }
    push(s, total);
  } else {
    printf("Error: Cannot sum, stack is full\n");
  }
}

// Push the sum of squares of all elements on the stack
void stack_sum_squares(Stack* s) {
  if (!isFull(s)) {
    double total = 0.0;
    for (int i = 0; i <= s->top; i++) {
      total += s->data[i] * s->data[i];
    }
    push(s, total);
  } else {
    printf("Error: Cannot sum squares, stack is full\n");
  }
}
