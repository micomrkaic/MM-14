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
#include "state.h"

typedef enum { REG_STO, REG_ADD, REG_SUB, REG_MUL, REG_DIV } RegOp;

int  reg_store(State *s, RegOp op);   /* register[reg_idx] op= X; X stays */
int  reg_recall(State *s);
int  reg_select(State *s);            /* pops index */
void reg_clear(State *s);
void reg_list(const State *s);
int  reg_save(const State *s);
int  reg_load(State *s, bool quiet_if_missing);

#endif
