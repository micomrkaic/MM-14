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
#include <string.h>
#include "ops.h"
#include "stack.h"
#include "my_math.h"
#include "my_statistics.h"
#include "registers.h"
#include "words.h"
#include "stack_printing.h"
#include "help_functions.h"
#include "numerics.h"

#define S (&s->stack)

static const char *cur_op = "";

/* ======================= generic helpers ======================= */

static int result(State *s, double r) {
  if (isnan(r)) return rpn_err("%s: invalid argument", cur_op);
  if (isinf(r)) return rpn_err("%s: result is infinite", cur_op);
  return push(S, r);
}

static int un(State *s, double (*f)(double)) {
  double x = pop(S);
  s->last_x = x;
  return result(s, f(x));
}

static int bin(State *s, double (*f)(double, double)) {
  double x = pop(S), y = pop(S);
  s->last_x = x;
  s->last_y = y;
  return result(s, f(y, x));
}

/* HP percent family: y stays in Y, x is replaced by the result. */
static int keep_y(State *s, double (*f)(double, double)) {
  double x = pop(S), y = peek(S, 0);
  s->last_x = x;
  return result(s, f(y, x));
}

static int tri(State *s, double (*f)(double, double, double)) {
  double x = pop(S), y = pop(S), z = pop(S);
  s->last_x = x;
  s->last_y = y;
  return result(s, f(z, y, x));
}

static int pop_index(State *s, int *out, int lo, int hi) {
  double v = pop(S);
  if (v != floor(v) || v < lo || v > hi)
    return rpn_err("%s: argument must be an integer in %d..%d", cur_op, lo, hi);
  *out = (int)v;
  return RPN_OK;
}

#define UN(nm, expr)                                                            \
  static double f_##nm(double x) { return (expr); }                            \
  static int op_##nm(State *s) { return un(s, f_##nm); }
#define BIN(nm, expr)                                                           \
  static double f_##nm(double y, double x) { (void)y; (void)x; return (expr); } \
  static int op_##nm(State *s) { return bin(s, f_##nm); }
#define KEEPY(nm, expr)                                                         \
  static double f_##nm(double y, double x) { return (expr); }                  \
  static int op_##nm(State *s) { return keep_y(s, f_##nm); }
#define TRI(nm, expr)                                                           \
  static double f_##nm(double z, double y, double x) { return (expr); }        \
  static int op_##nm(State *s) { return tri(s, f_##nm); }

/* ======================= arithmetic ======================= */
BIN(add, y + x)
BIN(sub, y - x)
BIN(mul, y * x)
BIN(div, y / x)
BIN(pow, pow(y, x))
BIN(mod, mod_floor(y, x))
BIN(min, fmin(y, x))
BIN(max, fmax(y, x))
BIN(gcd, gcd_d(y, x))
BIN(lcm, lcm_d(y, x))
BIN(ncr, n_choose_k(y, x))
BIN(npr, n_perm_k(y, x))
KEEPY(pct, y * x / 100.0)
KEEPY(pctt, 100.0 * x / y)
KEEPY(pctchg, 100.0 * (x - y) / y)

static int op_divide(State *s) {
  if (peek(S, 0) == 0.0) return rpn_err("Division by zero");
  return op_div(s);
}

UN(chs, -x)
UN(abs, fabs(x))
UN(inv_raw, 1.0 / x)
UN(sq, x * x)
UN(sqrt, sqrt(x))
UN(cbrt, cbrt(x))
UN(int, trunc(x))
UN(frac, x - trunc(x))
UN(floor, floor(x))
UN(ceil, ceil(x))
UN(round, round(x))
UN(sign, (double)((x > 0) - (x < 0)))
UN(fact, factorial(x))
UN(gamma, tgamma(x))
UN(lgamma, lgamma(x))

static int op_inv(State *s) {
  if (peek(S, 0) == 0.0) return rpn_err("Division by zero");
  return op_inv_raw(s);
}

/* ======================= exp / log ======================= */
UN(exp, exp(x))
UN(ln, log(x))
UN(log, log10(x))
UN(log2, log2(x))
UN(alog, pow(10.0, x))

/* ======================= trigonometry ======================= */
UN(sin, trig_sin(x, cfg.angle))
UN(cos, trig_cos(x, cfg.angle))
UN(tan, trig_tan(x, cfg.angle))
UN(asin, ang_from_rad(asin(x), cfg.angle))
UN(acos, ang_from_rad(acos(x), cfg.angle))
UN(atan, ang_from_rad(atan(x), cfg.angle))
BIN(atan2, ang_from_rad(atan2(y, x), cfg.angle))
BIN(hypot, hypot(y, x))
UN(sinh, sinh(x))
UN(cosh, cosh(x))
UN(tanh, tanh(x))
UN(asinh, asinh(x))
UN(acosh, acosh(x))
UN(atanh, atanh(x))
UN(todeg, x * (180.0 / PI_D))
UN(torad, x * (PI_D / 180.0))
UN(tohms, to_hms(x))
UN(fromhms, from_hms(x))

static int set_angle(AngleMode m) { cfg.angle = m; return RPN_OK; }
static int op_deg(State *s)  { (void)s; return set_angle(ANG_DEG); }
static int op_rad(State *s)  { (void)s; return set_angle(ANG_RAD); }
static int op_grad(State *s) { (void)s; return set_angle(ANG_GRAD); }

/* y x -> theta r   (HP ->P) */
static int op_topolar(State *s) {
  double x = pop(S), y = pop(S);
  s->last_x = x;
  s->last_y = y;
  TRY(result(s, ang_from_rad(atan2(y, x), cfg.angle)));
  return result(s, hypot(y, x));
}

/* theta r -> y x   (HP ->R) */
static int op_torect(State *s) {
  double r = pop(S), th = pop(S);
  s->last_x = r;
  s->last_y = th;
  TRY(result(s, r * trig_sin(th, cfg.angle)));
  return result(s, r * trig_cos(th, cfg.angle));
}

/* ======================= constants / random ======================= */
static int op_pi(State *s)   { return push(S, PI_D); }
static int op_e(State *s)    { return push(S, E_D); }
static int op_rand(State *s) { return push(S, rng_uniform()); }
static int op_seed(State *s) { rng_seed(pop(S)); return RPN_OK; }

/* ======================= probability ======================= */
UN(npdf, normal_pdf(x))
UN(ncdf, normal_cdf(x))
UN(nqnt, normal_quantile(x))
BIN(tpdf, t_pdf(y, x))
BIN(tcdf, t_cdf(y, x))
BIN(tqnt, t_quantile(y, x))
BIN(chipdf, chi2_pdf(y, x))
BIN(chicdf, chi2_cdf(y, x))
BIN(chiqnt, chi2_quantile(y, x))
TRI(fpdf, f_pdf(z, y, x))
TRI(fcdf, f_cdf(z, y, x))
TRI(fqnt, f_quantile(z, y, x))

/* ======================= tests & logic ======================= */
BIN(eq, (double)(y == x))
BIN(ne, (double)(y != x))
BIN(lt, (double)(y < x))
BIN(le, (double)(y <= x))
BIN(gt, (double)(y > x))
BIN(ge, (double)(y >= x))
BIN(and, (double)(y != 0.0 && x != 0.0))
BIN(or, (double)(y != 0.0 || x != 0.0))
UN(not, (double)(x == 0.0))

/* ======================= stack ======================= */
static int op_dup(State *s)   { return push(S, peek(S, 0)); }
static int op_drop(State *s)  { pop(S); return RPN_OK; }
static int op_2drop(State *s) { pop(S); pop(S); return RPN_OK; }
static int op_over(State *s)  { return push(S, peek(S, 1)); }
static int op_clx(State *s)   { S->data[S->top] = 0.0; return RPN_OK; }
static int op_clst(State *s)  { S->top = -1; return RPN_OK; }
static int op_depth(State *s) { return push(S, stack_depth(S)); }
static int op_lstx(State *s)  { return push(S, s->last_x); }
static int op_lsty(State *s)  { return push(S, s->last_y); }

static int op_swap(State *s) {
  double t = S->data[S->top];
  S->data[S->top] = S->data[S->top - 1];
  S->data[S->top - 1] = t;
  return RPN_OK;
}

static int op_2dup(State *s) {
  double a = peek(S, 1), b = peek(S, 0);
  TRY(push(S, a));
  return push(S, b);
}

static int op_nip(State *s) {
  double x = pop(S);
  S->data[S->top] = x;
  return RPN_OK;
}

/* a b -- b a b */
static int op_tuck(State *s) {
  double b = pop(S), a = pop(S);
  TRY(push(S, b));
  TRY(push(S, a));
  return push(S, b);
}

/* Move the item at depth n (0 = top) to the top. */
static void roll_n(Stack *st, int n) {
  int i = st->top - n;
  double v = st->data[i];
  memmove(&st->data[i], &st->data[i + 1], (size_t)n * sizeof(double));
  st->data[st->top] = v;
}

/* Move the top item down to depth n (inverse of roll_n). */
static void unroll_n(Stack *st, int n) {
  int i = st->top - n;
  double v = st->data[st->top];
  memmove(&st->data[i + 1], &st->data[i], (size_t)n * sizeof(double));
  st->data[i] = v;
}

static int op_rot(State *s)    { roll_n(S, 2); return RPN_OK; }
static int op_mrot(State *s)   { unroll_n(S, 2); return RPN_OK; }

/* For pick/roll the index itself is on the stack: after popping it,
   valid depths are 0 .. depth-2 (depth counted before the pop). */
static int op_roll(State *s) {
  int n = 0;
  if (stack_depth(S) < 2) { pop(S); return rpn_err("roll: no item to roll"); }
  TRY(pop_index(s, &n, 0, stack_depth(S) - 2));
  roll_n(S, n);
  return RPN_OK;
}

static int op_pick(State *s) {
  int n = 0;
  if (stack_depth(S) < 2) { pop(S); return rpn_err("pick: no item to pick"); }
  TRY(pop_index(s, &n, 0, stack_depth(S) - 2));
  return push(S, peek(S, n));
}

/* ======================= registers ======================= */
static int op_regidx(State *s)  { return reg_select(s); }
static int op_sto(State *s)     { return reg_store(s, REG_STO); }
static int op_stoadd(State *s)  { return reg_store(s, REG_ADD); }
static int op_stosub(State *s)  { return reg_store(s, REG_SUB); }
static int op_stomul(State *s)  { return reg_store(s, REG_MUL); }
static int op_stodiv(State *s)  { return reg_store(s, REG_DIV); }
static int op_rcl(State *s)     { return reg_recall(s); }
static int op_listregs(State *s){ reg_list(s); return RPN_OK; }
static int op_clrg(State *s)    { reg_clear(s); return RPN_OK; }
static int op_saveregs(State *s){ return reg_save(s); }
static int op_loadregs(State *s){ return reg_load(s, false); }

/* ======================= statistics ======================= */
/* HP order: x in X, y in Y. */
static int op_splus(State *s) {
  double x = pop(S), y = pop(S);
  s->last_x = x;
  s->last_y = y;
  sigma_add(&s->sig, x, y);
  return push(S, s->sig.n);
}

static int op_sminus(State *s) {
  double x = pop(S), y = pop(S);
  s->last_x = x;
  s->last_y = y;
  TRY(sigma_remove(&s->sig, x, y));
  return push(S, s->sig.n);
}

static int op_clstat(State *s) { sigma_clear(&s->sig); return RPN_OK; }

static int need_n(const State *s, double n) {
  if (s->sig.n < n) return rpn_err("%s: need at least %.0f data point%s", cur_op, n, n > 1 ? "s" : "");
  return RPN_OK;
}

/* Pushes y-value then x-value, so X holds the x statistic (HP convention). */
static int push_pair(State *s, double yv, double xv) {
  TRY(result(s, yv));
  return result(s, xv);
}

static int op_mean(State *s) {
  TRY(need_n(s, 1));
  SigmaOut o; sigma_derive(&s->sig, &o);
  return push_pair(s, o.my, o.mx);
}

static int op_sdev(State *s) {
  TRY(need_n(s, 2));
  SigmaOut o; sigma_derive(&s->sig, &o);
  return push_pair(s, o.sdy, o.sdx);
}

static int op_corr(State *s) {
  TRY(need_n(s, 2));
  SigmaOut o; sigma_derive(&s->sig, &o);
  return result(s, o.r);
}

/* HP-11C L.R.: slope in Y, intercept in X. */
static int op_lr(State *s) {
  TRY(need_n(s, 2));
  SigmaOut o; sigma_derive(&s->sig, &o);
  return push_pair(s, o.b, o.a);
}

static int op_yhat(State *s) {
  TRY(need_n(s, 2));
  SigmaOut o; sigma_derive(&s->sig, &o);
  double x = pop(S);
  s->last_x = x;
  return result(s, o.a + o.b * x);
}

static int op_xhat(State *s) {
  TRY(need_n(s, 2));
  SigmaOut o; sigma_derive(&s->sig, &o);
  double y = pop(S);
  s->last_x = y;
  if (o.b == 0.0) return rpn_err("xhat: regression slope is zero");
  return result(s, (y - o.a) / o.b);
}

static int op_statscalc(State *s) {
  SigmaOut o;
  sigma_derive(&s->sig, &o);
  char a[64], b[64];
#define ROW(l1, v1, l2, v2)                                     \
  do { fmt_num(a, sizeof a, v1); fmt_num(b, sizeof b, v2);      \
       printf("  %-6s %22s    %-6s %22s\n", l1, a, l2, b); } while (0)
  fmt_num(a, sizeof a, o.n);
  printf("  %-6s %22s\n", "n", a);
  ROW("mean x", o.mx, "mean y", o.my);
  ROW("s x", o.sdx, "s y", o.sdy);
  ROW("r", o.r, "r^2", o.r * o.r);
  ROW("a", o.a, "b", o.b);
  printf("  (sample std devs; y = a + b*x)\n");
#undef ROW
  return RPN_OK;
}

/* ======================= display ======================= */
static int op_p(State *s) {
  int n = 0;
  TRY(pop_index(s, &n, 0, MAX_PRECISION));
  cfg.precision = n;
  return RPN_OK;
}

static int set_disp(DispMode m) { cfg.disp = m; return RPN_OK; }
static int op_fix(State *s) { (void)s; return set_disp(DISP_FIX); }
static int op_sci(State *s) { (void)s; return set_disp(DISP_SCI); }
static int op_eng(State *s) { (void)s; return set_disp(DISP_ENG); }
static int op_all(State *s) { (void)s; return set_disp(DISP_ALL); }
static int op_spf(State *s) { (void)s; return set_disp((DispMode)((cfg.disp + 1) % 4)); }
static int op_sss(State *s) { (void)s; cfg.show_stat_regs = !cfg.show_stat_regs; return RPN_OK; }
static int op_swapterm(State *s) { (void)s; cfg.small_terminal = !cfg.small_terminal; return RPN_OK; }

static int op_dot(State *s) {
  char buf[64];
  fmt_num(buf, sizeof buf, pop(S));
  printf("%s\n", buf);
  return RPN_OK;
}

static int op_dots(State *s) {
  if (stack_depth(S) == 0) printf("  (empty)\n");
  else print_stack_plain(s);
  return RPN_OK;
}

/* ======================= words ======================= */
static int op_listwords(State *s)  { (void)s; list_words(); return RPN_OK; }
static int op_clearwords(State *s) { (void)s; return clear_words(); }
static int op_savewords(State *s)  { (void)s; return save_words(); }
static int op_loadwords(State *s)  { (void)s; return load_words(false); }
static int op_delword(State *s) {
  if (word_count == 0) { pop(S); return rpn_err("delword: no words defined"); }
  int i = 0;
  TRY(pop_index(s, &i, 0, word_count - 1));
  return delete_word(i);
}

/* ======================= session ======================= */
static int op_help(State *s) { (void)s; help_screen(); return RPN_OK; }
static int op_quit(State *s) { (void)s; cfg.quit = true; return RPN_OK; }

/* ======================= stack aggregates ======================= */
static int op_stacksum(State *s) {
  double acc = 0.0, comp = 0.0;
  for (int i = 0; i <= S->top; i++) {        /* Kahan */
    double y = S->data[i] - comp, t = acc + y;
    comp = (t - acc) - y;
    acc = t;
  }
  return result(s, acc);
}

static int op_stacksumsq(State *s) {
  double acc = 0.0;
  for (int i = 0; i <= S->top; i++) acc += S->data[i] * S->data[i];
  return result(s, acc);
}

static int op_stackmean(State *s) {
  double acc = 0.0;
  int n = stack_depth(S);
  for (int i = 0; i < n; i++) acc += (S->data[i] - acc) / (i + 1);
  return result(s, acc);
}

static int op_stackmax(State *s) {
  double m = S->data[0];
  for (int i = 1; i <= S->top; i++) m = fmax(m, S->data[i]);
  return result(s, m);
}

static int op_stackmin(State *s) {
  double m = S->data[0];
  for (int i = 1; i <= S->top; i++) m = fmin(m, S->data[i]);
  return result(s, m);
}

/* Feed every stack item into the statistics as an x value (y = 0); clears the stack. */
static int op_stack2stat(State *s) {
  for (int i = 0; i <= S->top; i++) sigma_add(&s->sig, S->data[i], 0.0);
  S->top = -1;
  return push(S, s->sig.n);
}

/* ======================= numerical methods ======================= */
static int op_exec(State *s) {
  double xt = pop(S);
  TRY(xt_check(xt));
  return xt_exec(s, xt, 1);
}

static int op_tol(State *s) {
  double t = pop(S);
  if (!(t > 0.0 && t < 1.0)) return rpn_err("tol: tolerance must be between 0 and 1");
  num_tol = t;
  return RPN_OK;
}

static int op_integrate(State *s) {
  double xt = pop(S), b = pop(S), a = pop(S), r, err;
  s->last_x = b; s->last_y = a;
  TRY(num_integrate(s, xt, a, b, &r, &err));
  return result(s, r);
}

static int op_romberg(State *s) {
  double xt = pop(S), b = pop(S), a = pop(S), r;
  s->last_x = b; s->last_y = a;
  TRY(num_romberg(s, xt, a, b, &r));
  return result(s, r);
}

static int op_fzero(State *s) {
  double xt = pop(S), b = pop(S), a = pop(S), r;
  s->last_x = b; s->last_y = a;
  TRY(num_fzero(s, xt, a, b, &r));
  return result(s, r);
}

static int op_solve(State *s) {
  double xt = pop(S), g = pop(S), r;
  s->last_x = g;
  TRY(num_solve(s, xt, g, &r));
  return result(s, r);
}

static int op_minimize(State *s) {
  double xt = pop(S), b = pop(S), a = pop(S), r;
  s->last_x = b; s->last_y = a;
  TRY(num_minimize(s, xt, a, b, &r));
  return result(s, r);
}

static int op_deriv(State *s) {
  double xt = pop(S), x = pop(S), r;
  s->last_x = x;
  TRY(num_deriv(s, xt, x, &r));
  return result(s, r);
}

static int op_fsum(State *s) {
  double xt = pop(S), hi = pop(S), lo = pop(S), r;
  s->last_x = hi; s->last_y = lo;
  TRY(num_fsum(s, xt, lo, hi, false, &r));
  return result(s, r);
}

static int op_fprod(State *s) {
  double xt = pop(S), hi = pop(S), lo = pop(S), r;
  s->last_x = hi; s->last_y = lo;
  TRY(num_fsum(s, xt, lo, hi, true, &r));
  return result(s, r);
}

static int op_nevals(State *s) { return push(S, (double)num_evals); }
static int op_interr(State *s) { return push(S, num_last_err); }

static int op_internals(State *s) {
  static const char *dn[] = { "FIX", "SCI", "ENG", "ALL" };
  static const char *an[] = { "RAD", "DEG", "GRAD" };
  printf("  display     %s, %d digits\n", dn[cfg.disp], cfg.precision);
  printf("  angle mode  %s\n", an[cfg.angle]);
  printf("  register    R%d selected\n", s->reg_idx);
  printf("  tol         %g   (integrate, fzero, solve, minimize)\n", num_tol);
  printf("  nevals      %ld  (function evaluations in the last numerical call)\n", num_evals);
  printf("  stack       %d of %d levels; undo %d of %d\n", stack_depth(S), MAX_STACK, undo_depth(), UNDO_LEVELS);
  printf("  words       %d defined\n", word_count);
  return RPN_OK;
}

/* ======================= the table ======================= */
#define SYN NULL
const Op ops[] = {
  /* Arithmetic */
  { "+",     2, op_add,    "Arithmetic", "y x -- y+x", "add" },
  { "-",     2, op_sub,    "Arithmetic", "y x -- y-x", "subtract" },
  { "*",     2, op_mul,    "Arithmetic", "y x -- y*x", "multiply" },
  { "/",     2, op_divide, "Arithmetic", "y x -- y/x", "divide" },
  { "^",     2, op_pow,    "Arithmetic", "y x -- y^x", "power" },
  { "pow",   2, op_pow,    "Arithmetic", "y x -- y^x", "power (alias of ^)" },
  { "chs",   1, op_chs,    "Arithmetic", "x -- -x", "change sign" },
  { "abs",   1, op_abs,    "Arithmetic", "x -- |x|", "absolute value" },
  { "inv",   1, op_inv,    "Arithmetic", "x -- 1/x", "reciprocal" },
  { "sq",    1, op_sq,     "Arithmetic", "x -- x^2", "square" },
  { "sqrt",  1, op_sqrt,   "Arithmetic", "x -- sqrt(x)", "square root" },
  { "cbrt",  1, op_cbrt,   "Arithmetic", "x -- cbrt(x)", "cube root" },
  { "%",     2, op_pct,    "Arithmetic", "y x -- y y*x/100", "percent of y (HP %; keeps y)" },
  { "%t",    2, op_pctt,   "Arithmetic", "y x -- y 100*x/y", "x as percent of total y (HP %T; keeps y)" },
  { "%chg",  2, op_pctchg, "Arithmetic", "y x -- y 100*(x-y)/y", "percent change from y to x (HP delta%; keeps y)" },
  { "mod",   2, op_mod,    "Arithmetic", "y x -- y mod x", "floored modulo (result has the sign of x)" },
  { "int",   1, op_int,    "Arithmetic", "x -- int(x)", "integer part (truncate toward zero)" },
  { "frac",  1, op_frac,   "Arithmetic", "x -- frac(x)", "fractional part" },
  { "floor", 1, op_floor,  "Arithmetic", "x -- floor(x)", "round toward -infinity" },
  { "ceil",  1, op_ceil,   "Arithmetic", "x -- ceil(x)", "round toward +infinity" },
  { "round", 1, op_round,  "Arithmetic", "x -- round(x)", "round half away from zero" },
  { "sign",  1, op_sign,   "Arithmetic", "x -- sgn(x)", "-1, 0 or 1" },
  { "min",   2, op_min,    "Arithmetic", "y x -- min", "smaller of y and x" },
  { "max",   2, op_max,    "Arithmetic", "y x -- max", "larger of y and x" },
  { "gcd",   2, op_gcd,    "Arithmetic", "y x -- gcd", "greatest common divisor (integers)" },
  { "lcm",   2, op_lcm,    "Arithmetic", "y x -- lcm", "least common multiple (integers)" },
  { "fact",  1, op_fact,   "Arithmetic", "x -- x!", "factorial, Gamma(x+1) for non-integers" },
  { "gamma", 1, op_gamma,  "Arithmetic", "x -- Gamma(x)", "gamma function" },
  { "lgamma",1, op_lgamma, "Arithmetic", "x -- ln|Gamma(x)|", "log-gamma" },
  { "ncr",   2, op_ncr,    "Arithmetic", "n k -- C(n,k)", "combinations" },
  { "npr",   2, op_npr,    "Arithmetic", "n k -- P(n,k)", "permutations" },

  /* Exponential & logarithm */
  { "exp",   1, op_exp,    "Exp/Log", "x -- e^x", "natural exponential" },
  { "ln",    1, op_ln,     "Exp/Log", "x -- ln(x)", "natural logarithm" },
  { "10^x",  1, op_alog,   "Exp/Log", "x -- 10^x", "common antilog" },
  { "log",   1, op_log,    "Exp/Log", "x -- log10(x)", "common logarithm" },
  { "log2",  1, op_log2,   "Exp/Log", "x -- log2(x)", "binary logarithm" },

  /* Trigonometry */
  { "sin",   1, op_sin,    "Trigonometry", "x -- sin(x)", "sine (current angle mode)" },
  { "cos",   1, op_cos,    "Trigonometry", "x -- cos(x)", "cosine" },
  { "tan",   1, op_tan,    "Trigonometry", "x -- tan(x)", "tangent" },
  { "asin",  1, op_asin,   "Trigonometry", "x -- asin(x)", "inverse sine" },
  { "acos",  1, op_acos,   "Trigonometry", "x -- acos(x)", "inverse cosine" },
  { "atan",  1, op_atan,   "Trigonometry", "x -- atan(x)", "inverse tangent" },
  { "atan2", 2, op_atan2,  "Trigonometry", "y x -- atan2(y,x)", "angle of the point (x,y)" },
  { "hypot", 2, op_hypot,  "Trigonometry", "y x -- sqrt(x^2+y^2)", "hypotenuse" },
  { "sinh",  1, op_sinh,   "Trigonometry", "x -- sinh(x)", "hyperbolic sine" },
  { "cosh",  1, op_cosh,   "Trigonometry", "x -- cosh(x)", "hyperbolic cosine" },
  { "tanh",  1, op_tanh,   "Trigonometry", "x -- tanh(x)", "hyperbolic tangent" },
  { "asinh", 1, op_asinh,  "Trigonometry", "x -- asinh(x)", "inverse hyperbolic sine" },
  { "acosh", 1, op_acosh,  "Trigonometry", "x -- acosh(x)", "inverse hyperbolic cosine" },
  { "atanh", 1, op_atanh,  "Trigonometry", "x -- atanh(x)", "inverse hyperbolic tangent" },
  { "deg",   0, op_deg,    "Trigonometry", "--", "angle mode: degrees" },
  { "rad",   0, op_rad,    "Trigonometry", "--", "angle mode: radians" },
  { "grad",  0, op_grad,   "Trigonometry", "--", "angle mode: grads" },
  { ">deg",  1, op_todeg,  "Trigonometry", "rad -- deg", "radians to degrees" },
  { ">rad",  1, op_torad,  "Trigonometry", "deg -- rad", "degrees to radians" },
  { ">p",    2, op_topolar,"Trigonometry", "y x -- theta r", "rectangular to polar (HP ->P)" },
  { ">r",    2, op_torect, "Trigonometry", "theta r -- y x", "polar to rectangular (HP ->R)" },
  { ">hms",  1, op_tohms,  "Trigonometry", "h -- H.MMSS", "decimal hours/degrees to H.MMSS" },
  { ">h",    1, op_fromhms,"Trigonometry", "H.MMSS -- h", "H.MMSS to decimal hours/degrees" },

  /* Constants & random */
  { "pi",    0, op_pi,     "Constants", "-- pi", "3.14159..." },
  { "e",     0, op_e,      "Constants", "-- e", "2.71828..." },
  { "rand",  0, op_rand,   "Constants", "-- u", "uniform random number in [0,1)" },
  { "seed",  1, op_seed,   "Constants", "x --", "seed the random number generator" },

  /* Probability */
  { "npdf",  1, op_npdf,   "Probability", "x -- pdf", "standard normal density" },
  { "ncdf",  1, op_ncdf,   "Probability", "x -- P", "standard normal CDF" },
  { "nqnt",  1, op_nqnt,   "Probability", "p -- x", "standard normal quantile" },
  { "tpdf",  2, op_tpdf,   "Probability", "t df -- pdf", "Student t density" },
  { "tcdf",  2, op_tcdf,   "Probability", "t df -- P", "Student t CDF" },
  { "tqnt",  2, op_tqnt,   "Probability", "p df -- t", "Student t quantile" },
  { "chipdf",2, op_chipdf, "Probability", "x k -- pdf", "chi-square density" },
  { "chicdf",2, op_chicdf, "Probability", "x k -- P", "chi-square CDF" },
  { "chiqnt",2, op_chiqnt, "Probability", "p k -- x", "chi-square quantile" },
  { "fpdf",  3, op_fpdf,   "Probability", "x d1 d2 -- pdf", "F density" },
  { "fcdf",  3, op_fcdf,   "Probability", "x d1 d2 -- P", "F CDF" },
  { "fqnt",  3, op_fqnt,   "Probability", "p d1 d2 -- x", "F quantile" },

  /* Tests & logic */
  { "==",    2, op_eq,     "Logic", "y x -- flag", "1 if y = x" },
  { "!=",    2, op_ne,     "Logic", "y x -- flag", "1 if y != x" },
  { "<",     2, op_lt,     "Logic", "y x -- flag", "1 if y < x" },
  { "<=",    2, op_le,     "Logic", "y x -- flag", "1 if y <= x" },
  { ">",     2, op_gt,     "Logic", "y x -- flag", "1 if y > x" },
  { ">=",    2, op_ge,     "Logic", "y x -- flag", "1 if y >= x" },
  { "&&",    2, op_and,    "Logic", "y x -- flag", "logical and" },
  { "||",    2, op_or,     "Logic", "y x -- flag", "logical or" },
  { "!",     1, op_not,    "Logic", "x -- flag", "logical not" },

  /* Stack */
  { "dup",   1, op_dup,    "Stack", "a -- a a", "duplicate X" },
  { "drop",  1, op_drop,   "Stack", "a --", "discard X" },
  { "swap",  2, op_swap,   "Stack", "a b -- b a", "exchange X and Y" },
  { "over",  2, op_over,   "Stack", "a b -- a b a", "copy Y to the top" },
  { "rot",   3, op_rot,    "Stack", "a b c -- b c a", "rotate the third item to the top" },
  { "-rot",  3, op_mrot,   "Stack", "a b c -- c a b", "rotate the top item to third" },
  { "nip",   2, op_nip,    "Stack", "a b -- b", "discard Y" },
  { "tuck",  2, op_tuck,   "Stack", "a b -- b a b", "copy X below Y" },
  { "pick",  1, op_pick,   "Stack", "... n -- ... v", "copy the item at depth n (0 pick = dup)" },
  { "roll",  1, op_roll,   "Stack", "... n -- ...", "move the item at depth n to the top (1 roll = swap)" },
  { "depth", 0, op_depth,  "Stack", "-- n", "number of items on the stack" },
  { "2dup",  2, op_2dup,   "Stack", "a b -- a b a b", "duplicate the top pair" },
  { "2drop", 2, op_2drop,  "Stack", "a b --", "discard the top pair" },
  { "clx",   1, op_clx,    "Stack", "x -- 0", "clear X to zero" },
  { "clst",  0, op_clst,   "Stack", "... --", "clear the whole stack" },
  { "lstx",  0, op_lstx,   "Stack", "-- last_x", "recall X before the last operation" },
  { "lsty",  0, op_lsty,   "Stack", "-- last_y", "recall Y before the last binary operation" },
  { "stacksize",0,op_depth, "Stack", "-- n", "number of items on the stack (alias of depth)" },
  { "stacksum",1,op_stacksum,"Stack", "... -- ... sum", "sum of all items (stack is kept)" },
  { "stacksumsq",1,op_stacksumsq,"Stack", "... -- ... sumsq", "sum of squares of all items" },
  { "stackmean",1,op_stackmean,"Stack", "... -- ... mean", "mean of all items" },
  { "stackmax",1,op_stackmax,"Stack", "... -- ... max", "largest item" },
  { "stackmin",1,op_stackmin,"Stack", "... -- ... min", "smallest item" },
  { "stack>stat",1,op_stack2stat,"Stack", "... -- n", "feed all items into the statistics as x values and clear the stack" },

  /* Registers */
  { "regidx",1, op_regidx, "Registers", "n --", "select register n (0..99) for sto/rcl" },
  { "sto",   1, op_sto,    "Registers", "x -- x", "store X in the selected register" },
  { "sto+",  1, op_stoadd, "Registers", "x -- x", "add X to the selected register" },
  { "sto-",  1, op_stosub, "Registers", "x -- x", "subtract X from the selected register" },
  { "sto*",  1, op_stomul, "Registers", "x -- x", "multiply the selected register by X" },
  { "sto/",  1, op_stodiv, "Registers", "x -- x", "divide the selected register by X" },
  { "rcl",   0, op_rcl,    "Registers", "-- v", "recall the selected register" },
  { "listregs",0,op_listregs,"Registers", "--", "list registers in use (* = selected)" },
  { "clrg",  0, op_clrg,   "Registers", "--", "clear all registers" },
  { "saveregs",0,op_saveregs,"Registers", "--", "save registers to the config directory" },
  { "loadregs",0,op_loadregs,"Registers", "--", "load registers from the config directory" },

  /* Statistics */
  { "s+",    2, op_splus,  "Statistics", "y x -- n", "accumulate the pair (x = X, y = Y)" },
  { "s-",    2, op_sminus, "Statistics", "y x -- n", "remove the pair (x = X, y = Y)" },
  { "clstat",0, op_clstat, "Statistics", "--", "clear statistics registers" },
  { "statscalc",0,op_statscalc,"Statistics", "--", "print summary statistics and regression" },
  { "mean",  0, op_mean,   "Statistics", "-- ybar xbar", "means" },
  { "sdev",  0, op_sdev,   "Statistics", "-- sy sx", "sample standard deviations" },
  { "corr",  0, op_corr,   "Statistics", "-- r", "correlation coefficient" },
  { "lr",    0, op_lr,     "Statistics", "-- b a", "linear regression y = a + b*x (slope in Y, intercept in X)" },
  { "yhat",  1, op_yhat,   "Statistics", "x -- yhat", "regression estimate of y" },
  { "xhat",  1, op_xhat,   "Statistics", "y -- xhat", "regression estimate of x" },

  /* Numerical methods: f is an execution token from ' name, for a word x -- f(x) */
  { "'",     0, SYN,       "Numerics", "' name -- xt", "push the execution token of a word or command" },
  { "exec",  1, op_exec,   "Numerics", "xt --", "execute an execution token" },
  { "integrate",3,op_integrate,"Numerics", "a b xt -- I", "integral of f over [a,b] (tanh-sinh; handles endpoint singularities)" },
  { "romberg",3,op_romberg, "Numerics", "a b xt -- I", "integral of f over [a,b] by Romberg extrapolation" },
  { "fzero", 3, op_fzero,  "Numerics", "a b xt -- x", "root of f in the bracket [a,b] (Brent)" },
  { "solve", 2, op_solve,  "Numerics", "guess xt -- x", "root of f near guess (HP-15C style: find a bracket, then Brent)" },
  { "minimize",3,op_minimize,"Numerics", "a b xt -- x", "argument of the minimum of f in [a,b] (Brent)" },
  { "deriv", 2, op_deriv,  "Numerics", "x xt -- f'(x)", "derivative of f at x (Ridders extrapolation)" },
  { "fsum",  3, op_fsum,   "Numerics", "lo hi xt -- S", "sum of f(i) for integer i from lo to hi" },
  { "fprod", 3, op_fprod,  "Numerics", "lo hi xt -- P", "product of f(i) for integer i from lo to hi" },
  { "tol",   1, op_tol,    "Numerics", "x --", "set the convergence tolerance (default 1e-10)" },
  { "nevals",0, op_nevals, "Numerics", "-- n", "function evaluations used by the last numerical call" },
  { "interr",0, op_interr, "Numerics", "-- e", "error estimate of the last integrate" },

  /* Display */
  { "p",     1, op_p,      "Display", "n --", "set displayed digits (0..15)" },
  { "fix",   0, op_fix,    "Display", "--", "fixed-point display" },
  { "sci",   0, op_sci,    "Display", "--", "scientific display" },
  { "eng",   0, op_eng,    "Display", "--", "engineering display (exponent multiple of 3)" },
  { "all",   0, op_all,    "Display", "--", "show all significant digits" },
  { "spf",   0, op_spf,    "Display", "--", "cycle FIX -> SCI -> ENG -> ALL" },
  { "sss",   0, op_sss,    "Display", "--", "toggle the statistics panel" },
  { "swapterm",0,op_swapterm,"Display", "--", "toggle compact (small terminal) display" },
  { ".",     1, op_dot,    "Display", "x --", "print and drop X" },
  { ".s",    0, op_dots,   "Display", "--", "print the stack" },

  /* User words */
  { ":",     0, SYN,       "Words", ": name ... ;", "start a definition (may span lines)" },
  { ";",     0, SYN,       "Words", "", "end a definition" },
  { "see",   0, SYN,       "Words", "see name", "show a word's definition or a command's help" },
  { "listwords",0,op_listwords,"Words", "--", "list user words with their indices" },
  { "delword",1,op_delword, "Words", "i --", "delete user word number i" },
  { "clearwords",0,op_clearwords,"Words", "--", "delete all user words" },
  { "savewords",0,op_savewords,"Words", "--", "save words to the config directory" },
  { "loadwords",0,op_loadwords,"Words", "--", "replace words with the saved set" },
  { "(",     0, SYN,       "Words", "( comment )", "comment (needs spaces around the parenthesis)" },
  { "\\",    0, SYN,       "Words", "\\ comment", "comment to end of line" },

  /* Control flow: in definitions, or at top level where the rest of the line is compiled */
  { "if",    0, SYN,       "Control", "flag --", "if ... [else ...] then: run if flag != 0 (upper case accepted)" },
  { "else",  0, SYN,       "Control", "", "alternative branch of if" },
  { "then",  0, SYN,       "Control", "", "end of if" },
  { "begin", 0, SYN,       "Control", "", "start of begin ... until / begin ... while ... repeat" },
  { "until", 0, SYN,       "Control", "flag --", "loop back to begin while flag = 0" },
  { "while", 0, SYN,       "Control", "flag --", "leave the loop if flag = 0" },
  { "repeat",0, SYN,       "Control", "", "jump back to begin" },
  { "do",    0, SYN,       "Control", "limit start --", "counted loop; skipped if start = limit" },
  { "loop",  0, SYN,       "Control", "", "index += 1; repeat while index < limit" },
  { "+loop", 0, SYN,       "Control", "n --", "index += n; repeat until index crosses limit" },
  { "i",     0, SYN,       "Control", "-- index", "innermost loop index" },
  { "j",     0, SYN,       "Control", "-- index", "next outer loop index" },

  /* Session */
  { "u",     0, SYN,       "Session", "", "undo (up to 64 levels; a user word is one step)" },
  { "internals",0,op_internals,"Session", "--", "show display, angle, register, tolerance and limits" },
  { "help",  0, op_help,   "Session", "--", "list commands" },
  { "q",     0, op_quit,   "Session", "--", "quit (also Ctrl-D)" },
};
#undef SYN

const int n_ops = (int)(sizeof ops / sizeof ops[0]);

int find_op(const char *name) {
  for (int i = 0; i < n_ops; i++)
    if (strcmp(ops[i].name, name) == 0) return i;
  return -1;
}

int call_op(State *s, int idx) {
  const Op *op = &ops[idx];
  if (!op->fn) return rpn_err("'%s' can only be used inside a definition", op->name);
  if (stack_depth(S) < op->nargs)
    return rpn_err("%s: needs %d argument%s on the stack", op->name, op->nargs,
                   op->nargs == 1 ? "" : "s");
  const char *saved = cur_op;
  cur_op = op->name;
  int rc = op->fn(s);
  cur_op = saved;
  return rc;
}
