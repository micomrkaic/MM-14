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

#include <stdbool.h>
#include <math.h>
#include "stack.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_ROMBERG_ITER 20
#define MAX_BISECTION_ITER 100

double normal_pdf(double x);
double normal_cdf(double x);
double inverse_normal_cdf(double p, double tol);
double romberg(double (*f)(double), double a, double b, double tol, int max_iter);
void set_int_precision(Stack *stack);
void set_f0_precision(Stack *stack);
double integrate(double a, double b);
double romberg_helper(double x);
double find_zero(double a, double b);
bool bisection(double (*f)(double), double a, double b, double tol, double* root);


#endif // MY_MATH_H
