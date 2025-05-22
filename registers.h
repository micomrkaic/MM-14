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
#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdbool.h>
#include "stack.h"

#define MAX_REGS 64
extern double registers[MAX_REGS];
extern bool reg_used[MAX_REGS];  // track which registers are initialized

void clear_registers(void);
int save_registers_to_file(void);
int load_registers_from_file(void);
void list_registers(void);
void top_to_register(Stack *stack);
void register_plus_top(Stack *stack);
void register_minus_top(Stack *stack);
void register_times_top(Stack *stack);
void register_divided_by_top(Stack *stack);
void register_to_top(Stack *stack);
void set_reg_index(Stack* stack);

#endif // REGISTERS_H
