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
#include <stdio.h>
#include "words.h"
#include "my_math.h"
#include "globals.h"
#include "interpreter.h"

// **** Numerical functions **** 
double normal_pdf(double x) {
  return exp(-0.5 * x * x) / sqrt(2.0 * M_PI);
}

double normal_cdf(double x) {
  return 0.5 * (1.0 + erf(x / sqrt(2.0)));
}

double inverse_normal_cdf(double p, double tol) {
  if (p <= 0.0 || p >= 1.0) {
    fprintf(stderr, "p must be in (0,1)\n");
    return NAN;
  }

  double lo = -10, hi = 10, mid;

  while ((hi - lo) > tol) {
    mid = 0.5 * (lo + hi);
    double cdf_mid = normal_cdf(mid);
    if (cdf_mid < p) {
      lo = mid;
    } else {
      hi = mid;
    }
  }
  return 0.5 * (lo + hi);
}

// Function to integrate
double f(double x) {
  return sin(x);  // Change this to any integrable function
}

// Romberg integration
double romberg(double (*f)(double), double a, double b, double tol, int max_iter) {
  double R[MAX_ROMBERG_ITER][MAX_ROMBERG_ITER];
  int i, j;
  double h = b - a;

  // R[0][0] is the trapezoidal rule with 1 interval
  R[0][0] = 0.5 * h * (f(a) + f(b));

  for (i = 1; i < max_iter; i++) {
    h *= 0.5;

    // Compute trapezoidal sum for 2^i intervals
    double sum = 0.0;
    int n = 1 << (i - 1); // 2^(i-1)
    for (int k = 1; k <= n; k++) {
      sum += f(a + (2 * k - 1) * h);
    }
    
    R[i][0] = 0.5 * R[i - 1][0] + h * sum;
    
    // Richardson extrapolation
    for (j = 1; j <= i; j++) {
      R[i][j] = R[i][j - 1] + (R[i][j - 1] - R[i - 1][j - 1]) / (pow(4, j) - 1);
    }
    
    // Check for convergence
    if (i > 1 && fabs(R[i][i] - R[i - 1][i - 1]) < tol) {
      return R[i][i];
    }
  }
  
  printf("Warning: Romberg integration did not converge after %d iterations\n", max_iter);
  return R[max_iter - 1][max_iter - 1];
}
void set_int_precision(Stack *stack) {
  intg_tolerance = pop(stack);
}

void set_f0_precision(Stack *stack) {
  fsolve_tolerance = pop(stack);
}

double romberg_helper(double x) {
  Stack integration_stack;
  Stack old_integration_stack;

  initStack(&integration_stack);
  copyStack(&old_integration_stack, &integration_stack);

  push(&integration_stack,x);
  return evaluate_input(words[intg_function].name, &integration_stack, &old_integration_stack); 
}

double integrate(double a, double b) {
  return romberg(romberg_helper,a,b,intg_tolerance,MAX_ROMBERG_ITER);
}

double find_zero(double a, double b) {
  double root;
  if (bisection(romberg_helper, a, b, fsolve_tolerance, &root))
    return root;
  else
    return a;
}

// Bisection function
bool bisection(double (*f)(double), double a, double b, double tol, double* root) {
    double fa = f(a);
    double fb = f(b);

    // Check for a sign change
    if (fa * fb > 0) {
        printf("Error: f(a) and f(b) do not have opposite signs. No guaranteed root in [%.6f, %.6f]\n", a, b);
        return false;
    }

    for (int i = 0; i < MAX_BISECTION_ITER; ++i) {
        double mid = 0.5 * (a + b);
        double fmid = f(mid);

        if (fabs(fmid) < tol || (b - a) < tol) {
            *root = mid;
            return true;
        }

        // Determine subinterval
        if (fa * fmid < 0) {
            b = mid;
            fb = fmid;
        } else {
            a = mid;
            fa = fmid;
        }
    }
    printf("Warning: Maximum iterations reached without convergence\n");
    return false;
}
