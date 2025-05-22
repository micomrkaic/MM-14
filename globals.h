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
#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>

extern int precision;
extern int reg_idx;
extern double intg_tolerance;
extern int intg_function;
extern double fsolve_tolerance;
extern double previous_element;
extern int no_repeats;
extern bool terminate_program;
extern double last_x, last_y;
extern double Reg_stat_in[6], Reg_stat_out[6];
extern const char *stat_in_labels[6];
extern const char *stat_out_labels[6];
extern bool show_stat_regs;
extern bool fixed_point_format;
extern bool stack_underflow;
extern bool small_terminal;

#endif

