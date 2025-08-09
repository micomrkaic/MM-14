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

#include <stdio.h>      // FILE, fprintf, perror, snprintf
#include <stdlib.h>     // getenv
#include <string.h>     // snprintf (some libcs), memset
#include <sys/types.h>  // mode_t (portable mkdir prototype)
#include <sys/stat.h>   // mkdir
#include <errno.h>      // errno, EEXIST
#include <stdbool.h>    // bool, true/false (since you use bool)
#include "globals.h"
#include "registers.h"
#include "stack.h"

double registers[MAX_REGS];
bool reg_used[MAX_REGS];  // track which registers are initialized

// Clear registers
void clear_registers(void) {
  for(int i=0; i < MAX_REGS; i++) reg_used[i]=false;
}

/* mkdir -p for ~/.config and ~/.config/mm_14 */
static int ensure_mm14_config_dir(void) {
  const char *home = getenv("HOME");
  if (!home || !*home) { fprintf(stderr, "HOME not set\n"); return -1; }

  char p1[512], p2[512];
  snprintf(p1, sizeof p1, "%s/.config", home);
  snprintf(p2, sizeof p2, "%s/.config/mm_14", home);

  if (mkdir(p1, 0700) && errno != EEXIST) { perror("mkdir ~/.config"); return -1; }
  if (mkdir(p2, 0700) && errno != EEXIST) { perror("mkdir ~/.config/mm_14"); return -1; }
  return 0;
}

/* Build ~/.config/mm_14/<filename> into out */
static int get_mm14_config_path(const char *filename, char *out, size_t outsz) {
  const char *home = getenv("HOME");
  if (!home || !*home) { fprintf(stderr, "HOME not set\n"); return -1; }
  if (snprintf(out, outsz, "%s/.config/mm_14/%s", home, filename) >= (int)outsz) {
    fprintf(stderr, "Path too long\n"); return -1;
  }
  return 0;
}

int save_registers_to_file(void) {
  if (ensure_mm14_config_dir() != 0) return -1;

  char path[512];
  if (get_mm14_config_path("registers.txt", path, sizeof path) != 0) return -1;

  FILE *f = fopen(path, "w");
  if (!f) { perror("open for write registers.txt"); return -1; }

  for (int i = 0; i < MAX_REGS; i++) {
    if (reg_used[i]) {
      /* 17 sig figs prints a double round-trip safe */
      fprintf(f, "%d %.*g\n", i, 17, registers[i]);
    }
  }
  fclose(f);
  return 0;
}

int load_registers_from_file(void) {
  char path[512];
  if (get_mm14_config_path("registers.txt", path, sizeof path) != 0) return -1;

  FILE *f = fopen(path, "r");
  if (!f) { perror("open for read registers.txt"); return -1; }

  /* Clear all registers first */
  for (int i = 0; i < MAX_REGS; i++) {
    reg_used[i] = false;
    registers[i] = 0.0;
  }

  int reg;
  double val;
  while (fscanf(f, "%d %lf", &reg, &val) == 2) {
    if (reg >= 0 && reg < MAX_REGS) {
      registers[reg] = val;
      reg_used[reg] = true;
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
