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
#include <math.h>
#include <string.h>
#include "my_statistics.h"

void sigma_clear(Sigma *g) {
  memset(g, 0, sizeof *g);
}

/* Welford's online update; numerically stable, no catastrophic cancellation. */
void sigma_add(Sigma *g, double x, double y) {
  g->n += 1.0;
  double dx = x - g->mx;
  g->mx += dx / g->n;
  double dy = y - g->my;
  g->my += dy / g->n;
  g->m2x += dx * (x - g->mx);
  g->m2y += dy * (y - g->my);
  g->cxy += dx * (y - g->my);
}

/* Exact inverse of sigma_add. */
int sigma_remove(Sigma *g, double x, double y) {
  if (g->n < 1.0) return rpn_err("s-: statistics registers are empty");
  if (g->n == 1.0) { sigma_clear(g); return RPN_OK; }
  double n1 = g->n - 1.0;
  double mx_new = g->mx - (x - g->mx) / n1;
  double my_new = g->my - (y - g->my) / n1;
  g->m2x -= (x - mx_new) * (x - g->mx);
  g->m2y -= (y - my_new) * (y - g->my);
  g->cxy -= (x - mx_new) * (y - g->my);
  if (g->m2x < 0.0) g->m2x = 0.0;
  if (g->m2y < 0.0) g->m2y = 0.0;
  g->mx = mx_new;
  g->my = my_new;
  g->n = n1;
  return RPN_OK;
}

void sigma_derive(const Sigma *g, SigmaOut *o) {
  double n = g->n;
  o->n   = n;
  o->sx  = n * g->mx;
  o->sy  = n * g->my;
  o->sx2 = g->m2x + n * g->mx * g->mx;
  o->sy2 = g->m2y + n * g->my * g->my;
  o->sxy = g->cxy + n * g->mx * g->my;
  o->mx  = n >= 1.0 ? g->mx : NAN;
  o->my  = n >= 1.0 ? g->my : NAN;
  o->sdx = n >= 2.0 ? sqrt(g->m2x / (n - 1.0)) : NAN;
  o->sdy = n >= 2.0 ? sqrt(g->m2y / (n - 1.0)) : NAN;
  o->r   = (n >= 2.0 && g->m2x > 0.0 && g->m2y > 0.0) ? g->cxy / sqrt(g->m2x * g->m2y) : NAN;
  o->b   = (n >= 2.0 && g->m2x > 0.0) ? g->cxy / g->m2x : NAN;
  o->a   = isnan(o->b) ? NAN : g->my - o->b * g->mx;
}
