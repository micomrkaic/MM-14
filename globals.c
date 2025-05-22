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
#include "globals.h"


int precision = 6;  // For floating point output
int reg_idx = 0;    // Storage register address
double intg_tolerance = 1.0e-4;  // Integration tolerance
int intg_function = 0;  // The number of the word describing the function to integrate
double fsolve_tolerance = 1e-6;  // Zero finding tolerance
int no_repeats = 0; // For iterations
bool terminate_program = false;
double last_x = 0.0, last_y = 0.0;
double Reg_stat_in[6] = {0}, Reg_stat_out[6] = {0};
const char *stat_in_labels[6]  = { "N  ", "Σx ", "Σx²", "Σy ", "Σy²", "Σxy" };
const char *stat_out_labels[6] = { "N", " x̄", " σₓ", " ȳ", " σᵧ", " ρ" };
bool show_stat_regs = true;
bool fixed_point_format = true;
bool stack_underflow = false;
bool small_terminal = false;
