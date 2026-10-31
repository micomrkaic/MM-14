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
#ifndef MY_MATH_H
#define MY_MATH_H

#include "state.h"

#define PI_D 3.14159265358979323846
#define E_D  2.71828182845904523536

/* All functions return NaN for invalid arguments; callers turn that into an error. */

/* Distributions */
double normal_pdf(double x);
double normal_cdf(double x);
double normal_quantile(double p);           /* Acklam + Halley step, full precision */
double reg_gamma_p(double a, double x);     /* regularized lower incomplete gamma */
double reg_beta(double x, double a, double b);
double t_pdf(double t, double nu);
double t_cdf(double t, double nu);
double t_quantile(double p, double nu);
double chi2_pdf(double x, double k);
double chi2_cdf(double x, double k);
double chi2_quantile(double p, double k);
double f_pdf(double x, double d1, double d2);
double f_cdf(double x, double d1, double d2);
double f_quantile(double p, double d1, double d2);

/* Combinatorics / number theory */
double factorial(double x);                 /* x! = Gamma(x+1); exact for integers <= 170 */
double n_choose_k(double n, double k);
double n_perm_k(double n, double k);
double gcd_d(double a, double b);
double lcm_d(double a, double b);
double mod_floor(double y, double x);       /* result has the sign of x */

/* Time / angles */
double to_hms(double hours);                /* decimal hours -> H.MMSS */
double from_hms(double hms);                /* H.MMSS -> decimal hours */
double ang_to_rad(double x, AngleMode m);
double ang_from_rad(double r, AngleMode m);
double trig_sin(double x, AngleMode m);     /* exact at multiples of 90 deg / 100 grad */
double trig_cos(double x, AngleMode m);
double trig_tan(double x, AngleMode m);

/* Random numbers (xorshift64*) */
void   rng_seed(double x);
void   rng_seed_u64(unsigned long long v);
double rng_uniform(void);

#endif
