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
//#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "util_func.h"

// Check if string is a number
bool isNumber(const char *str) {
  if (!str || *str == '\0') return false;
  char *endptr;

  // Check for integer first
  strtol(str, &endptr, 10);
  if (*endptr == '\0') return true;

  // Check for valid float (including scientific notation)
  strtod(str, &endptr);
  if (*endptr == '\0') return true;
  return false;
}

double safe_div(double a, double b) {
  if (b == 0.0) {
    printf("Error: Division by zero!\n");
    return 0.0;
  }
  return a / b;
}

double safe_ln(double a)
{
  if (a <= 0) {
    printf("Error: ln of non-positive number!\n");
    return 0;
  }
  return log(a); 
}

double safe_log10(double a)
{
  if (a <= 0) {
    printf("Error: log of non-positive number!\n");
    return 0.0;
  }
  return log10(a);
}

double safe_frac(double a)
{
  double intpart;
  return modf(a, &intpart);
}

double safe_int(double a)
{
  double intpart;
  modf(a, &intpart);
  return intpart;
}

void set_disp_precision(Stack* stack)
{
  if (stack->top < 0) {
    printf("Error: stack underflow — no value for precision\n");
    return;
  }
  double val = pop(stack);
  // Check if val is a valid integer precision 0–9
  if (val >= 0 && val <= 9 && floor(val) == val) {
    precision = (int)val;
    printf("Precision set to %d decimal places\n", precision);
  } else {
    printf("Invalid precision value (must be integer from 0 to 9)\n");
  }
  return;
}
void print_internals(void) {
  printf(" **** Control variables ****\n");
  printf("Print precision:%d\n",  precision);
  printf("Storage register address: %d\n",reg_idx);
  printf("Integration tolerance: %g\n",intg_tolerance);
  printf("Fsolve tolerance: %g\n",fsolve_tolerance);
  printf("Number of function to analyze:%d\n",intg_function);
  printf("\n");
}
