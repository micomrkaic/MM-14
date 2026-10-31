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
#ifndef NUMERICS_H
#define NUMERICS_H

#include "state.h"

/* Numerical methods over user functions. A function is an execution token
   (from ' name) for a word that maps x on the stack to f(x) on the stack.
   Every routine evaluates the function on the live State, so words may use
   registers; an error inside the function aborts the routine and the caller
   rolls the whole thing back. Convergence is governed by cfg.tol. */

#define DEFAULT_TOL   1e-10
#define MAX_FN_EVALS  2000000

extern double num_tol;          /* relative/absolute tolerance, set by 'tol' */
extern long   num_evals;        /* function evaluations in the last call */
extern double num_last_err;     /* error estimate of the last integrate */

int num_eval(State *s, double xt, double x, double *fx);

int num_integrate(State *s, double xt, double a, double b, double *result, double *err);
int num_romberg(State *s, double xt, double a, double b, double *result);
int num_fzero(State *s, double xt, double a, double b, double *root);
int num_solve(State *s, double xt, double guess, double *root);
int num_minimize(State *s, double xt, double a, double b, double *xmin);
int num_deriv(State *s, double xt, double x, double *dfdx);
int num_fsum(State *s, double xt, double lo, double hi, bool product, double *result);

#endif
