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
#ifndef UTIL_FUNC_H
#define UTIL_FUNC_H

#include <stdbool.h>
#include "stack.h"
#include "globals.h"

bool isNumber(const char *str);
double safe_div(double a, double b);
double safe_ln(double a);
double safe_log10(double a);
double safe_frac(double a);
double safe_int(double a);
void set_disp_precision(Stack* stack);
void print_internals(void);
#endif // UTIL_FUNC_H
