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

#include <stddef.h>
#include "state.h"

/* Format according to cfg (FIX/SCI/ENG/ALL, precision). Never overflows buf. */
void fmt_num(char *buf, size_t n, double x);

/* Full display after each interactive line (big or compact, plus status line). */
void print_stack(const State *s);

/* One value per line, deepest first (".s" and batch output). */
void print_stack_plain(const State *s);

/* Display width of a UTF-8 string (all our labels are single-width). */
size_t utf8_width(const char *s);

#endif
