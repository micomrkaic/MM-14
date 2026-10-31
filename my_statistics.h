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
#ifndef MY_STATISTICS_H
#define MY_STATISTICS_H

#include "state.h"

/* Everything derivable from the accumulators. NaN where undefined. */
typedef struct {
  double n, sx, sx2, sy, sy2, sxy;   /* HP-style sums */
  double mx, my, sdx, sdy, r;        /* means, sample std devs, correlation */
  double a, b;                       /* regression y = a + b x */
} SigmaOut;

void sigma_clear(Sigma *g);
void sigma_add(Sigma *g, double x, double y);
int  sigma_remove(Sigma *g, double x, double y);   /* RPN_ERR if n == 0 */
void sigma_derive(const Sigma *g, SigmaOut *o);

#endif
