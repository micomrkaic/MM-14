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
#include <stddef.h>

typedef enum { NUM_NO = 0, NUM_OK = 1, NUM_RANGE = -1 } NumParse;

/* Decimal literals only: digits, sign, '.', exponent. No nan/inf/hex. */
NumParse parse_number(const char *s, double *out);

/* $XDG_CONFIG_HOME/mm_14/<file>, falling back to ~/.config/mm_14/<file>.
   With create=true the directory is made if missing. */
int config_path(const char *file, char *out, size_t outsz, bool create);

#endif
