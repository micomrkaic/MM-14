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
#include <float.h>
#include <math.h>
#include <stdio.h>
#include "numerics.h"
#include "stack.h"
#include "words.h"

double num_tol = DEFAULT_TOL;
long   num_evals = 0;

/* ======================= function evaluation ======================= */

int num_eval(State *s, double xt, double x, double *fx) {
  if (interrupted) return rpn_err("Interrupted");
  if (++num_evals > MAX_FN_EVALS)
    return rpn_err("%s: more than %d function evaluations; loosen tol", xt_name(xt), MAX_FN_EVALS);
  int depth0 = stack_depth(&s->stack);
  TRY(push(&s->stack, x));
  if (xt_exec(s, xt, 1) != RPN_OK)
    return rpn_err("  while evaluating %s at x = %.17g", xt_name(xt), x);
  int d = stack_depth(&s->stack) - depth0;
  if (d != 1) {
    /* leave the stack as we found it so the caller's rollback message is clean */
    while (stack_depth(&s->stack) > depth0) pop(&s->stack);
    return rpn_err("%s must leave exactly one result (x -- f(x)); it left %d", xt_name(xt), d);
  }
  *fx = pop(&s->stack);
  if (isnan(*fx)) return rpn_err("%s(%.17g) is not a number", xt_name(xt), x);
  return RPN_OK;
}

static int check_interval(const char *what, double a, double b) {
  if (!isfinite(a) || !isfinite(b)) return rpn_err("%s: limits must be finite", what);
  return RPN_OK;
}

/* ======================= tanh-sinh quadrature ======================= */
/* Double-exponential rule: x = c + d*tanh(pi/2 sinh(t)), t = k*h. Nodes
   cluster at the endpoints, so integrable endpoint singularities and
   infinite derivatives are handled; convergence is exponential for
   analytic integrands. Levels halve h; each level reuses the previous sum.
   Interior peaks defeat it, so when a segment fails it is bisected. */

double num_last_err = 0.0;

/* One segment. *ok says whether the tolerance (or the roundoff floor) was reached. */
static int ts_segment(State *s, double xt, double a, double b, int maxlevel,
                      double *result, double *err, bool *ok) {
  const double c = 0.5 * (a + b), d = 0.5 * (b - a);
  const double pio2 = 1.5707963267948966;
  double fc;
  TRY(num_eval(s, xt, c, &fc));
  double sum = pio2 * fc;           /* the t = 0 node; d*h is applied at the end */
  double h = 1.0, prev = 0.0, prev_err = INFINITY;
  *ok = false;
  *err = INFINITY;

  for (int level = 0; level <= maxlevel; level++) {
    if (level > 0) h *= 0.5;
    /* level 0 adds every multiple of h; later levels add the odd multiples
       of the new, halved h, reusing the sum over the old nodes */
    double step = level == 0 ? h : 2.0 * h;
    for (double t = h; t < 7.0; t += step) {
      double u = pio2 * sinh(t);
      double eu = exp(-2.0 * u);
      double one_minus_tanh = 2.0 * eu / (1.0 + eu);
      double dist = d * one_minus_tanh;                       /* distance from the endpoint */
      double w = pio2 * cosh(t) * 2.0 * one_minus_tanh / (1.0 + eu);   /* pi/2 cosh t / cosh^2 u */
      if (w < DBL_MIN) break;
      double xl = a + dist, xr = b - dist;
      /* Each side stops on its own when its node rounds onto the endpoint:
         a singularity at 0 keeps its nodes down to 1e-308. */
      bool left = xl > a, right = xr < b;
      if (!left && !right) break;
      double fl = 0.0, fr = 0.0;
      if (left) TRY(num_eval(s, xt, xl, &fl));
      if (right) TRY(num_eval(s, xt, xr, &fr));
      double contrib = w * (fl + fr);
      sum += contrib;
      if (t > 3.0 && fabs(contrib) < 1e-18 * fabs(sum)) break;
    }
    double cur = d * h * sum;
    if (level > 0) {
      *err = fabs(cur - prev);
      double scale = fmax(1.0, fabs(cur));
      if (*err <= num_tol * scale) { *result = cur; *ok = true; return RPN_OK; }
      /* Convergence should be super-geometric. When successive differences
         stop shrinking fast while already small, roundoff (an endpoint
         singularity away from 0 rounds its nodes onto the endpoint) is the
         limit; accept what we have. */
      if (level >= 3 && *err > 0.3 * prev_err && *err <= 1e4 * num_tol * scale) {
        *result = cur; *ok = true; return RPN_OK;
      }
      prev_err = *err;
    }
    prev = cur;
  }
  *result = prev;
  return RPN_OK;
}

static int ts_adaptive(State *s, double xt, double a, double b, int depth,
                       double *result, double *err) {
  bool ok;
  TRY(ts_segment(s, xt, a, b, 7, result, err, &ok));
  if (ok) return RPN_OK;
  if (depth >= 16)
    return rpn_err("integrate: no convergence on [%.17g, %.17g] (error estimate %g); "
                   "the integrand may be too rough or non-integrable", a, b, *err);
  double m = 0.5 * (a + b), r1, r2, e1, e2;
  TRY(ts_adaptive(s, xt, a, m, depth + 1, &r1, &e1));
  TRY(ts_adaptive(s, xt, m, b, depth + 1, &r2, &e2));
  *result = r1 + r2;
  *err = e1 + e2;
  return RPN_OK;
}

int num_integrate(State *s, double xt, double a, double b, double *result, double *err) {
  TRY(check_interval("integrate", a, b));
  num_evals = 0;
  TRY(xt_check(xt));
  num_last_err = 0.0;
  if (a == b) { *result = 0.0; *err = 0.0; return RPN_OK; }
  double sign = 1.0;
  if (a > b) { double t = a; a = b; b = t; sign = -1.0; }
  bool ok;
  TRY(ts_segment(s, xt, a, b, 12, result, err, &ok));
  if (!ok) TRY(ts_adaptive(s, xt, a, b, 0, result, err));
  *result *= sign;
  num_last_err = *err;
  if (*err > num_tol * fmax(1.0, fabs(*result))) {
    fflush(stdout);
    fprintf(stderr, "Warning: integrate: error estimate %.2g exceeds tol %g "
                    "(an endpoint singularity converges fully only at x = 0)\n", *err, num_tol);
  }
  return RPN_OK;
}

/* ======================= Romberg ======================= */

int num_romberg(State *s, double xt, double a, double b, double *result) {
  TRY(check_interval("romberg", a, b));
  num_evals = 0;
  TRY(xt_check(xt));
  enum { MAXK = 22 };
  double R[MAXK][MAXK];
  double fa, fb, hh = b - a;
  TRY(num_eval(s, xt, a, &fa));
  TRY(num_eval(s, xt, b, &fb));
  R[0][0] = 0.5 * hh * (fa + fb);
  for (int i = 1; i < MAXK; i++) {
    hh *= 0.5;
    long n = 1L << (i - 1);
    double sum = 0.0;
    for (long k = 1; k <= n; k++) {
      double f;
      TRY(num_eval(s, xt, a + (2 * k - 1) * hh, &f));
      sum += f;
    }
    R[i][0] = 0.5 * R[i - 1][0] + hh * sum;
    double p4 = 4.0;
    for (int j = 1; j <= i; j++, p4 *= 4.0)
      R[i][j] = R[i][j - 1] + (R[i][j - 1] - R[i - 1][j - 1]) / (p4 - 1.0);
    if (i >= 2 && fabs(R[i][i] - R[i - 1][i - 1]) < num_tol * fmax(1.0, fabs(R[i][i]))) {
      *result = R[i][i];
      return RPN_OK;
    }
  }
  *result = R[MAXK - 1][MAXK - 1];
  return rpn_err("romberg: no convergence to tol %g after %d levels", num_tol, MAXK);
}

/* ======================= Brent root finding ======================= */

static int brent_root(State *s, double xt, double a, double b, double fa, double fb, double *root) {
  if (fa == 0.0) { *root = a; return RPN_OK; }
  if (fb == 0.0) { *root = b; return RPN_OK; }
  if ((fa < 0) == (fb < 0)) return rpn_err("fzero: f(%g) and f(%g) have the same sign", a, b);
  double c = a, fc = fa, d = b - a, e = d;
  for (int it = 0; it < 200; it++) {
    if ((fb < 0) == (fc < 0)) { c = a; fc = fa; d = e = b - a; }
    if (fabs(fc) < fabs(fb)) { a = b; b = c; c = a; fa = fb; fb = fc; fc = fa; }
    double tol1 = 2.0 * DBL_EPSILON * fabs(b) + 0.5 * num_tol;
    double xm = 0.5 * (c - b);
    if (fabs(xm) <= tol1 || fb == 0.0) { *root = b; return RPN_OK; }
    if (fabs(e) >= tol1 && fabs(fa) > fabs(fb)) {
      double p, q, r, sgn = fb / fa;
      if (a == c) { p = 2.0 * xm * sgn; q = 1.0 - sgn; }
      else {
        q = fa / fc; r = fb / fc;
        p = sgn * (2.0 * xm * q * (q - r) - (b - a) * (r - 1.0));
        q = (q - 1.0) * (r - 1.0) * (sgn - 1.0);
      }
      if (p > 0) q = -q;
      p = fabs(p);
      if (2.0 * p < fmin(3.0 * xm * q - fabs(tol1 * q), fabs(e * q))) { e = d; d = p / q; }
      else { d = xm; e = d; }
    } else { d = xm; e = d; }
    a = b; fa = fb;
    b += (fabs(d) > tol1) ? d : (xm > 0 ? tol1 : -tol1);
    TRY(num_eval(s, xt, b, &fb));
  }
  *root = b;
  return rpn_err("fzero: no convergence after 200 iterations");
}

int num_fzero(State *s, double xt, double a, double b, double *root) {
  TRY(check_interval("fzero", a, b));
  num_evals = 0;
  TRY(xt_check(xt));
  double fa, fb;
  TRY(num_eval(s, xt, a, &fa));
  TRY(num_eval(s, xt, b, &fb));
  return brent_root(s, xt, a, b, fa, fb, root);
}

/* One initial guess, as on the HP-15C: expand a bracket around it
   geometrically (also following a secant step), then hand over to Brent. */
int num_solve(State *s, double xt, double g, double *root) {
  if (!isfinite(g)) return rpn_err("solve: guess must be finite");
  num_evals = 0;
  TRY(xt_check(xt));
  double fg;
  TRY(num_eval(s, xt, g, &fg));
  if (fg == 0.0) { *root = g; return RPN_OK; }
  double h = fmax(fabs(g) * 0.05, 0.05);
  double a = g, b = g, fa = fg, fb = fg;
  for (int i = 0; i < 80; i++) {
    a = g - h; b = g + h;
    TRY(num_eval(s, xt, a, &fa));
    TRY(num_eval(s, xt, b, &fb));
    if ((fa < 0) != (fg < 0)) return brent_root(s, xt, a, g, fa, fg, root);
    if ((fb < 0) != (fg < 0)) return brent_root(s, xt, g, b, fg, fb, root);
    /* secant step from the two points with the smaller |f| as a guess at the root */
    if (fb != fa) {
      double xs = a - fa * (b - a) / (fb - fa), fs;
      if (isfinite(xs) && fabs(xs - g) < 1e6 * h) {
        TRY(num_eval(s, xt, xs, &fs));
        if (fs == 0.0) { *root = xs; return RPN_OK; }
        if ((fs < 0) != (fg < 0)) {
          return (xs < g) ? brent_root(s, xt, xs, g, fs, fg, root)
                          : brent_root(s, xt, g, xs, fg, fs, root);
        }
      }
    }
    h *= 1.8;
  }
  return rpn_err("solve: no sign change found near %g; try fzero with a bracket", g);
}

/* ======================= Brent minimisation ======================= */

int num_minimize(State *s, double xt, double a, double b, double *xmin) {
  TRY(check_interval("minimize", a, b));
  num_evals = 0;
  TRY(xt_check(xt));
  if (a > b) { double t = a; a = b; b = t; }
  const double cgold = 0.3819660112501051;
  double x, w, v, fx, fw, fv, d = 0.0, e = 0.0;
  x = w = v = a + cgold * (b - a);
  TRY(num_eval(s, xt, x, &fx));
  fw = fv = fx;
  for (int it = 0; it < 500; it++) {
    double xm = 0.5 * (a + b);
    double tol1 = sqrt(fmax(num_tol, DBL_EPSILON)) * fabs(x) + num_tol / 3.0;
    double tol2 = 2.0 * tol1;
    if (fabs(x - xm) <= tol2 - 0.5 * (b - a)) { *xmin = x; return RPN_OK; }
    bool golden = true;
    if (fabs(e) > tol1) {
      double r = (x - w) * (fx - fv), q = (x - v) * (fx - fw);
      double p = (x - v) * q - (x - w) * r;
      q = 2.0 * (q - r);
      if (q > 0) p = -p;
      q = fabs(q);
      double etemp = e;
      e = d;
      if (fabs(p) < fabs(0.5 * q * etemp) && p > q * (a - x) && p < q * (b - x)) {
        d = p / q;
        double u = x + d;
        if (u - a < tol2 || b - u < tol2) d = (xm - x >= 0) ? tol1 : -tol1;
        golden = false;
      }
    }
    if (golden) { e = (x >= xm) ? a - x : b - x; d = cgold * e; }
    double u = (fabs(d) >= tol1) ? x + d : x + (d >= 0 ? tol1 : -tol1), fu;
    TRY(num_eval(s, xt, u, &fu));
    if (fu <= fx) {
      if (u >= x) a = x; else b = x;
      v = w; fv = fw; w = x; fw = fx; x = u; fx = fu;
    } else {
      if (u < x) a = u; else b = u;
      if (fu <= fw || w == x) { v = w; fv = fw; w = u; fw = fu; }
      else if (fu <= fv || v == x || v == w) { v = u; fv = fu; }
    }
  }
  *xmin = x;
  return rpn_err("minimize: no convergence after 500 iterations");
}

/* ======================= derivative (Ridders) ======================= */

int num_deriv(State *s, double xt, double x, double *dfdx) {
  if (!isfinite(x)) return rpn_err("deriv: x must be finite");
  num_evals = 0;
  TRY(xt_check(xt));
  enum { NTAB = 10 };
  const double con = 1.4, con2 = con * con, safe = 2.0;
  double a[NTAB][NTAB];
  double hh = 0.1 * fmax(fabs(x), 1.0), err = DBL_MAX, ans = 0.0;
  double fp, fm;
  TRY(num_eval(s, xt, x + hh, &fp));
  TRY(num_eval(s, xt, x - hh, &fm));
  a[0][0] = (fp - fm) / (2.0 * hh);
  for (int i = 1; i < NTAB; i++) {
    hh /= con;
    TRY(num_eval(s, xt, x + hh, &fp));
    TRY(num_eval(s, xt, x - hh, &fm));
    a[0][i] = (fp - fm) / (2.0 * hh);
    double fac = con2;
    for (int j = 1; j <= i; j++) {
      a[j][i] = (a[j - 1][i] * fac - a[j - 1][i - 1]) / (fac - 1.0);
      fac *= con2;
      double errt = fmax(fabs(a[j][i] - a[j - 1][i]), fabs(a[j][i] - a[j - 1][i - 1]));
      if (errt <= err) { err = errt; ans = a[j][i]; }
    }
    if (fabs(a[i][i] - a[i - 1][i - 1]) >= safe * err) break;
  }
  if (!isfinite(ans)) return rpn_err("deriv: derivative is not finite");
  *dfdx = ans;
  return RPN_OK;
}

/* ======================= sums and products ======================= */

int num_fsum(State *s, double xt, double lo, double hi, bool product, double *result) {
  const char *what = product ? "fprod" : "fsum";
  if (lo != floor(lo) || hi != floor(hi) || !isfinite(lo) || !isfinite(hi))
    return rpn_err("%s: limits must be integers", what);
  if (hi - lo > 1e7) return rpn_err("%s: more than 1e7 terms", what);
  num_evals = 0;
  TRY(xt_check(xt));
  double acc = product ? 1.0 : 0.0, comp = 0.0;
  for (double i = lo; i <= hi; i += 1.0) {
    double f;
    TRY(num_eval(s, xt, i, &f));
    if (product) acc *= f;
    else {                          /* Kahan summation */
      double y = f - comp, t = acc + y;
      comp = (t - acc) - y;
      acc = t;
    }
  }
  if (!isfinite(acc)) return rpn_err("%s: result is not finite", what);
  *result = acc;
  return RPN_OK;
}
