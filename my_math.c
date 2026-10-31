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
#include <stdint.h>
#include <string.h>
#include "my_math.h"

#define FPMIN 1e-300
#define CF_EPS 1e-16
#define CF_MAXIT 20000

/* ================= Normal ================= */
double normal_pdf(double x) {
  return exp(-0.5 * x * x) / sqrt(2.0 * PI_D);
}

double normal_cdf(double x) {
  return 0.5 * erfc(-x / sqrt(2.0));
}

/* Acklam's rational approximation (rel. error 1.15e-9), then one Halley step
   against erfc, which brings it to full double precision. Lower half only;
   the upper half uses symmetry (1-p is exact for p >= 0.5). */
static double nq_lower(double p) {
  static const double a[] = { -3.969683028665376e+01,  2.209460984245205e+02,
                              -2.759285104469687e+02,  1.383577518672690e+02,
                              -3.066479806614716e+01,  2.506628277459239e+00 };
  static const double b[] = { -5.447609879822406e+01,  1.615858368580409e+02,
                              -1.556989798598866e+02,  6.680131188771972e+01,
                              -1.328068155288572e+01 };
  static const double c[] = { -7.784894002430293e-03, -3.223964580411365e-01,
                              -2.400758277161838e+00, -2.549732539343734e+00,
                               4.374664141464968e+00,  2.938163982698783e+00 };
  static const double d[] = {  7.784695709041462e-03,  3.224671290700398e-01,
                               2.445134137142996e+00,  3.754408661907416e+00 };
  const double plow = 0.02425;
  double x;
  if (p < plow) {
    double q = sqrt(-2.0 * log(p));
    x = (((((c[0]*q + c[1])*q + c[2])*q + c[3])*q + c[4])*q + c[5]) /
        ((((d[0]*q + d[1])*q + d[2])*q + d[3])*q + 1.0);
  } else {
    double q = p - 0.5, r = q * q;
    x = (((((a[0]*r + a[1])*r + a[2])*r + a[3])*r + a[4])*r + a[5]) * q /
        (((((b[0]*r + b[1])*r + b[2])*r + b[3])*r + b[4])*r + 1.0);
  }
  double e = normal_cdf(x) - p;
  double u = e * sqrt(2.0 * PI_D) * exp(0.5 * x * x);
  return x - u / (1.0 + 0.5 * x * u);
}

double normal_quantile(double p) {
  if (!(p > 0.0 && p < 1.0)) return NAN;
  if (p == 0.5) return 0.0;
  return (p < 0.5) ? nq_lower(p) : -nq_lower(1.0 - p);
}

/* ================= Incomplete gamma ================= */
static double gamma_series(double a, double x) {
  double ap = a, sum = 1.0 / a, del = sum;
  for (int n = 0; n < CF_MAXIT; n++) {
    ap += 1.0;
    del *= x / ap;
    sum += del;
    if (fabs(del) < fabs(sum) * CF_EPS) break;
  }
  return sum * exp(-x + a * log(x) - lgamma(a));
}

static double gamma_cf(double a, double x) {   /* returns Q(a,x) */
  double b = x + 1.0 - a, c = 1.0 / FPMIN, d = 1.0 / b, h = d;
  for (int i = 1; i < CF_MAXIT; i++) {
    double an = -i * (i - a);
    b += 2.0;
    d = an * d + b; if (fabs(d) < FPMIN) d = FPMIN;
    c = b + an / c; if (fabs(c) < FPMIN) c = FPMIN;
    d = 1.0 / d;
    double del = d * c;
    h *= del;
    if (fabs(del - 1.0) < CF_EPS) break;
  }
  return exp(-x + a * log(x) - lgamma(a)) * h;
}

double reg_gamma_p(double a, double x) {
  if (!(a > 0.0) || isnan(x)) return NAN;
  if (x <= 0.0) return 0.0;
  if (isinf(x)) return 1.0;
  return (x < a + 1.0) ? gamma_series(a, x) : 1.0 - gamma_cf(a, x);
}

/* ================= Incomplete beta ================= */
static double beta_cf(double a, double b, double x) {
  double qab = a + b, qap = a + 1.0, qam = a - 1.0;
  double c = 1.0, d = 1.0 - qab * x / qap;
  if (fabs(d) < FPMIN) d = FPMIN;
  d = 1.0 / d;
  double h = d;
  for (int m = 1; m <= CF_MAXIT; m++) {
    int m2 = 2 * m;
    double aa = m * (b - m) * x / ((qam + m2) * (a + m2));
    d = 1.0 + aa * d; if (fabs(d) < FPMIN) d = FPMIN;
    c = 1.0 + aa / c; if (fabs(c) < FPMIN) c = FPMIN;
    d = 1.0 / d;
    h *= d * c;
    aa = -(a + m) * (qab + m) * x / ((a + m2) * (qap + m2));
    d = 1.0 + aa * d; if (fabs(d) < FPMIN) d = FPMIN;
    c = 1.0 + aa / c; if (fabs(c) < FPMIN) c = FPMIN;
    d = 1.0 / d;
    double del = d * c;
    h *= del;
    if (fabs(del - 1.0) < CF_EPS) break;
  }
  return h;
}

double reg_beta(double x, double a, double b) {
  if (!(a > 0.0 && b > 0.0) || isnan(x)) return NAN;
  if (x <= 0.0) return 0.0;
  if (x >= 1.0) return 1.0;
  double lbt = lgamma(a + b) - lgamma(a) - lgamma(b) + a * log(x) + b * log1p(-x);
  double bt = exp(lbt);
  if (x < (a + 1.0) / (a + b + 2.0)) return bt * beta_cf(a, b, x) / a;
  return 1.0 - bt * beta_cf(b, a, 1.0 - x) / b;
}

/* ================= Generic monotone CDF inversion ================= */
typedef double (*Cdf)(double x, const double *par);

static double invert_cdf(Cdf cdf, const double *par, double p, double lo, double hi,
                         bool lo_fixed) {
  if (!(p > 0.0 && p < 1.0)) return NAN;
  for (int i = 0; i < 2000 && cdf(hi, par) < p; i++) { lo = hi; hi = 2.0 * hi + 1.0; }
  if (!lo_fixed)
    for (int i = 0; i < 2000 && cdf(lo, par) > p; i++) { hi = lo; lo = 2.0 * lo - 1.0; }
  for (int i = 0; i < 3000; i++) {
    double mid = 0.5 * (lo + hi);
    if (mid <= lo || mid >= hi) break;
    if (cdf(mid, par) < p) lo = mid; else hi = mid;
  }
  return 0.5 * (lo + hi);
}

/* ================= Student t ================= */
double t_pdf(double t, double nu) {
  if (!(nu > 0.0) || isnan(t)) return NAN;
  return exp(lgamma(0.5 * (nu + 1.0)) - lgamma(0.5 * nu) - 0.5 * log(nu * PI_D)
             - 0.5 * (nu + 1.0) * log1p(t * t / nu));
}

double t_cdf(double t, double nu) {
  if (!(nu > 0.0) || isnan(t)) return NAN;
  double t2 = t * t;
  if (t2 < nu) {
    double h = 0.5 * reg_beta(t2 / (nu + t2), 0.5, 0.5 * nu);
    return t >= 0.0 ? 0.5 + h : 0.5 - h;
  }
  double tail = 0.5 * reg_beta(nu / (nu + t2), 0.5 * nu, 0.5);
  return t >= 0.0 ? 1.0 - tail : tail;
}

static double t_cdf_p(double x, const double *p) { return t_cdf(x, p[0]); }

double t_quantile(double p, double nu) {
  if (!(nu > 0.0)) return NAN;
  if (p == 0.5) return 0.0;
  double par[1] = { nu };
  return invert_cdf(t_cdf_p, par, p, -1.0, 1.0, false);
}

/* ================= Chi-square ================= */
double chi2_pdf(double x, double k) {
  if (!(k > 0.0) || isnan(x)) return NAN;
  if (x < 0.0) return 0.0;
  if (x == 0.0) return (k < 2.0) ? INFINITY : (k == 2.0 ? 0.5 : 0.0);
  double h = 0.5 * k;
  return exp((h - 1.0) * log(x) - 0.5 * x - h * log(2.0) - lgamma(h));
}

double chi2_cdf(double x, double k) {
  if (!(k > 0.0) || isnan(x)) return NAN;
  return (x <= 0.0) ? 0.0 : reg_gamma_p(0.5 * k, 0.5 * x);
}

static double chi2_cdf_p(double x, const double *p) { return chi2_cdf(x, p[0]); }

double chi2_quantile(double p, double k) {
  if (!(k > 0.0)) return NAN;
  double par[1] = { k };
  return invert_cdf(chi2_cdf_p, par, p, 0.0, k > 1.0 ? k : 1.0, true);
}

/* ================= F ================= */
double f_pdf(double x, double d1, double d2) {
  if (!(d1 > 0.0 && d2 > 0.0) || isnan(x)) return NAN;
  if (x < 0.0) return 0.0;
  if (x == 0.0) return (d1 < 2.0) ? INFINITY : (d1 == 2.0 ? 1.0 : 0.0);
  double lbeta = lgamma(0.5 * d1) + lgamma(0.5 * d2) - lgamma(0.5 * (d1 + d2));
  double l = 0.5 * (d1 * log(d1 * x) + d2 * log(d2) - (d1 + d2) * log(d1 * x + d2))
             - log(x) - lbeta;
  return exp(l);
}

double f_cdf(double x, double d1, double d2) {
  if (!(d1 > 0.0 && d2 > 0.0) || isnan(x)) return NAN;
  if (x <= 0.0) return 0.0;
  return reg_beta(d1 * x / (d1 * x + d2), 0.5 * d1, 0.5 * d2);
}

static double f_cdf_p(double x, const double *p) { return f_cdf(x, p[0], p[1]); }

double f_quantile(double p, double d1, double d2) {
  if (!(d1 > 0.0 && d2 > 0.0)) return NAN;
  double par[2] = { d1, d2 };
  return invert_cdf(f_cdf_p, par, p, 0.0, 1.0, true);
}

/* ================= Combinatorics ================= */
static bool is_int(double x) { return isfinite(x) && x == floor(x); }

double factorial(double x) {
  if (is_int(x)) {
    if (x < 0.0) return NAN;
    if (x > 170.0) return INFINITY;
    double r = 1.0;
    for (int i = 2; i <= (int)x; i++) r *= i;
    return r;
  }
  return tgamma(x + 1.0);
}

double n_choose_k(double n, double k) {
  if (!is_int(n) || !is_int(k) || k < 0.0 || n < 0.0 || k > n) return NAN;
  if (k > n - k) k = n - k;
  double r = 1.0;
  for (double i = 1.0; i <= k; i += 1.0) {
    r = r * (n - k + i) / i;
    if (isinf(r)) return r;
  }
  return (r < 9007199254740992.0) ? round(r) : r;
}

double n_perm_k(double n, double k) {
  if (!is_int(n) || !is_int(k) || k < 0.0 || n < 0.0 || k > n) return NAN;
  double r = 1.0;
  for (double i = n - k + 1.0; i <= n; i += 1.0) {
    r *= i;
    if (isinf(r)) return r;
  }
  return r;
}

double gcd_d(double a, double b) {
  if (!is_int(a) || !is_int(b) || fabs(a) > 9007199254740992.0 || fabs(b) > 9007199254740992.0)
    return NAN;
  a = fabs(a); b = fabs(b);
  while (b != 0.0) { double t = fmod(a, b); a = b; b = t; }
  return a;
}

double lcm_d(double a, double b) {
  double g = gcd_d(a, b);
  if (isnan(g)) return NAN;
  if (g == 0.0) return 0.0;
  return fabs(a / g * b);
}

double mod_floor(double y, double x) {
  if (x == 0.0) return NAN;
  double r = fmod(y, x);
  if (r != 0.0 && ((r < 0.0) != (x < 0.0))) r += x;
  return r;
}

/* ================= H.MS ================= */
double to_hms(double h) {
  double sgn = h < 0 ? -1.0 : 1.0;
  h = fabs(h);
  double hh = floor(h);
  double rem = (h - hh) * 60.0;
  double mm = floor(rem + 1e-9);
  double ss = (rem - mm) * 60.0;
  if (ss < 0.0) ss = 0.0;
  return sgn * (hh + mm / 100.0 + ss / 10000.0);
}

double from_hms(double x) {
  double sgn = x < 0 ? -1.0 : 1.0;
  x = fabs(x);
  double hh = floor(x);
  double f = (x - hh) * 100.0;
  double mm = floor(f + 1e-9);
  double ss = (f - mm) * 100.0;
  if (ss < 0.0) ss = 0.0;
  return sgn * (hh + mm / 60.0 + ss / 3600.0);
}

/* ================= Angles ================= */
static double full_turn(AngleMode m) {
  return m == ANG_DEG ? 360.0 : m == ANG_GRAD ? 400.0 : 2.0 * PI_D;
}

double ang_to_rad(double x, AngleMode m) {
  return m == ANG_DEG ? x * (PI_D / 180.0) : m == ANG_GRAD ? x * (PI_D / 200.0) : x;
}

double ang_from_rad(double r, AngleMode m) {
  return m == ANG_DEG ? r * (180.0 / PI_D) : m == ANG_GRAD ? r * (200.0 / PI_D) : r;
}

/* Quarter-turn index 0..3 if x is an exact multiple of 90 deg / 100 grad, else -1. */
static int quadrant(double x, AngleMode m) {
  if (m == ANG_RAD || !isfinite(x)) return -1;
  double k = x / (full_turn(m) / 4.0);
  if (k != floor(k)) return -1;
  double r = fmod(k, 4.0);
  if (r < 0.0) r += 4.0;
  return (int)r;
}

static double reduce(double x, AngleMode m) {
  return m == ANG_RAD ? x : fmod(x, full_turn(m));
}

double trig_sin(double x, AngleMode m) {
  static const double v[4] = { 0.0, 1.0, 0.0, -1.0 };
  int q = quadrant(x, m);
  return q >= 0 ? v[q] : sin(ang_to_rad(reduce(x, m), m));
}

double trig_cos(double x, AngleMode m) {
  static const double v[4] = { 1.0, 0.0, -1.0, 0.0 };
  int q = quadrant(x, m);
  return q >= 0 ? v[q] : cos(ang_to_rad(reduce(x, m), m));
}

double trig_tan(double x, AngleMode m) {
  int q = quadrant(x, m);
  if (q == 0 || q == 2) return 0.0;
  if (q == 1 || q == 3) return NAN;   /* pole */
  return tan(ang_to_rad(reduce(x, m), m));
}

/* ================= RNG ================= */
static uint64_t rng_state = 0x9E3779B97F4A7C15ULL;

static uint64_t splitmix64(uint64_t x) {
  x += 0x9E3779B97F4A7C15ULL;
  x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
  x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
  return x ^ (x >> 31);
}

void rng_seed_u64(unsigned long long v) {
  rng_state = splitmix64((uint64_t)v);
  if (rng_state == 0) rng_state = 1;
}

void rng_seed(double x) {
  uint64_t v;
  memcpy(&v, &x, sizeof v);
  rng_seed_u64(v);
}

double rng_uniform(void) {
  rng_state ^= rng_state >> 12;
  rng_state ^= rng_state << 25;
  rng_state ^= rng_state >> 27;
  return (double)((rng_state * 2685821657736338717ULL) >> 11) * 0x1.0p-53;
}
