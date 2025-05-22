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
#ifndef STACK_PRINTING_H
#define STACK_PRINTING_H

#include "stack.h"
#include "globals.h"

#define TERM_WIDTH 90

void printStack_small_terminal(Stack* s, int precision, char* stack_title);
void printStack_big_terminal(Stack* s, int precision);
void printStack(Stack* s, int precision, char* stack_title);
#endif // STACK_PRINTING_H
