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
#include "registers.h"
#include "stack.h"

double registers[MAX_REGS];
bool reg_used[MAX_REGS];  // track which registers are initialized

// Clear registers
void clear_registers(void) {
  for(int i=0; i < MAX_REGS; i++) reg_used[i]=false;
}

int save_registers_to_file(void) {
  FILE* f = fopen("registers.txt", "w");
  if (!f) {
    perror("Could not open registers.txt for writing");
    return -1;
  }

  for (int i = 0; i < MAX_REGS; i++) {
    if (reg_used[i]) {
      fprintf(f, "%d %.*g\n", i, 17, registers[i]);
    }
  }

  fclose(f);
  return 0;
}

int load_registers_from_file(void) {
  FILE* f = fopen("registers.txt", "r");
  if (!f) {
    perror("Could not open registers.txt for reading");
    return -1;
  }

  // Clear all registers first
  for (int i = 0; i < MAX_REGS; i++) {
    reg_used[i] = false;
    registers[i] = 0.0;
  }

  int reg;
  double val;

  while (fscanf(f, " %d %lf", &reg, &val) == 2) {
    if (reg >= 0 && reg <= MAX_REGS) {
      int idx = reg;
      registers[idx] = val;
      reg_used[idx] = true;
    } else {
      fprintf(stderr, "Invalid register number: %d\n", reg);
    }
  }

  fclose(f);
  return 0;
}

void list_registers(void) {
  printf("Occupied registers:\n");
  bool any = false;
  for (int i = 0; i < MAX_REGS; i++) {
    if (reg_used[i]) {
      if (fixed_point_format)
	printf("  %2d <- %*.*f\n", i, precision + 4, precision, registers[i]);
      else
	printf("  %2d <- %*.*e\n", i, precision + 4, precision, registers[i]);
      any = true;
    }
  }
  if (!any) {
    printf("  (none)\n");
  }
}

void top_to_register(Stack *stack)
{
  if (reg_idx < 0 || reg_idx >= MAX_REGS) {
    printf("Error: invalid or unset register index (reg_idx = %d)\n", reg_idx);
  } else if (stack->top < 0) {
    printf("Error: stack underflow — nothing to store\n");
  } else {
    registers[reg_idx] = stack->data[stack->top];
    reg_used[reg_idx] = true;
    printf("Stored in register %d\n", reg_idx);
  }
  return;
}

void register_plus_top(Stack *stack)
{
  if (reg_idx < 0 || reg_idx >= MAX_REGS) {
    printf("Error: invalid or unset register index (reg_idx = %d)\n", reg_idx);
  } else if (stack->top < 0) {
    printf("Error: stack underflow — nothing to store\n");
  } else {
    registers[reg_idx] += stack->data[stack->top];
    reg_used[reg_idx] = true;
    printf("Stored in register %d\n", reg_idx);
  }
  return;
}

void register_minus_top(Stack *stack)
{
  if (reg_idx < 0 || reg_idx >= MAX_REGS) {
    printf("Error: invalid or unset register index (reg_idx = %d)\n", reg_idx);
  } else if (stack->top < 0) {
    printf("Error: stack underflow — nothing to store\n");
  } else {
    registers[reg_idx] -= stack->data[stack->top];
    reg_used[reg_idx] = true;
    printf("Stored in register %d\n", reg_idx);
  }
  return;
}

void register_times_top(Stack *stack)
{
  if (reg_idx < 0 || reg_idx >= MAX_REGS) {
    printf("Error: invalid or unset register index (reg_idx = %d)\n", reg_idx);
  } else if (stack->top < 0) {
    printf("Error: stack underflow — nothing to store\n");
  } else {
    registers[reg_idx] *= stack->data[stack->top];
    reg_used[reg_idx] = true;
    printf("Stored in register %d\n", reg_idx);
  }
  return;
}

void register_divided_by_top(Stack *stack)
{
  if (reg_idx < 0 || reg_idx >= MAX_REGS) {
    printf("Error: invalid or unset register index (reg_idx = %d)\n", reg_idx);
  } else if (stack->top < 0) {
    printf("Error: stack underflow — nothing to store\n");
  } else {
    registers[reg_idx] /= stack->data[stack->top];
    reg_used[reg_idx] = true;
    printf("Stored in register %d\n", reg_idx);
  }
  return;
}

void register_to_top(Stack *stack)
{
  if (reg_idx < 0 || reg_idx >= MAX_REGS) {
    printf("Error: invalid or unset register index (reg_idx = %d)\n", reg_idx);
  } else if (!reg_used[reg_idx]) {
    printf("Register %c is empty\n", 'a' + reg_idx);
  } else if (stack->top >= MAX_STACK - 1) {
    printf("Stack overflow — cannot push value from register\n");
  } else {
    push(stack, registers[reg_idx]);
    printf("Recalled %g from register %c\n", registers[reg_idx], 'a' + reg_idx);
  }
  return;
}

void set_reg_index(Stack* stack)
{
  if (stack->top < 0) {
    printf("Error: stack underflow\n");
    return;
  }
  int idx = (int)pop(stack);
  // Check if val is a valid register adderess
  if (idx >= 0 && idx <= MAX_REGS) {
    reg_idx  = idx;
  } else {
    printf("Invalid register address\n");
  }
  return;
}
