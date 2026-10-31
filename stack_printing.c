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
#include <stdlib.h>
#include <string.h>
#include "stack_printing.h"
#include "stack.h"
#include "my_statistics.h"

/* ======================= number formatting ======================= */

static void fmt_eng(char *buf, size_t n, double x, int p) {
  if (x == 0.0) { snprintf(buf, n, "%.*fe+00", p, 0.0); return; }
  int e = (int)floor(log10(fabs(x)));
  int e3 = e - (((e % 3) + 3) % 3);
  char m[64];
  snprintf(m, sizeof m, "%.*f", p, x / pow(10.0, e3));
  if (fabs(atof(m)) >= 1000.0) {       /* rounding carried into the next group */
    e3 += 3;
    snprintf(m, sizeof m, "%.*f", p, x / pow(10.0, e3));
  }
  snprintf(buf, n, "%se%+03d", m, e3);
}

void fmt_num(char *buf, size_t n, double x) {
  int p = cfg.precision;
  if (!isfinite(x)) { snprintf(buf, n, "---"); return; }
  if (x == 0.0) x = 0.0;               /* no "-0.000" */
  switch (cfg.disp) {
  case DISP_FIX:
    /* Like the HP: fall back to SCI when FIX would overflow or show only zeros. */
    if (x != 0.0 && (fabs(x) >= 1e12 || fabs(x) < 0.5 * pow(10.0, -p)))
      snprintf(buf, n, "%.*e", p, x);
    else
      snprintf(buf, n, "%.*f", p, x);
    break;
  case DISP_SCI: snprintf(buf, n, "%.*e", p, x); break;
  case DISP_ENG: fmt_eng(buf, n, x, p); break;
  case DISP_ALL: snprintf(buf, n, "%.15g", x); break;
  }
}

/* Counts code points, skipping combining diacritics U+0300..U+036F
   (the macron in "x̄" is one: x + U+0304). */
size_t utf8_width(const char *s) {
  size_t w = 0;
  const unsigned char *p = (const unsigned char *)s;
  while (*p) {
    if ((p[0] == 0xCC && (p[1] & 0xC0) == 0x80) || (p[0] == 0xCD && p[1] >= 0x80 && p[1] <= 0xAF)) {
      p += 2;
      continue;
    }
    if ((*p & 0xC0) != 0x80) w++;
    p++;
  }
  return w;
}

/* ======================= line builder ======================= */

typedef struct { char buf[1024]; size_t len, col; } Line;

static void put(Line *l, const char *s) {
  size_t k = strlen(s);
  if (l->len + k >= sizeof l->buf) k = sizeof l->buf - 1 - l->len;
  memcpy(l->buf + l->len, s, k);
  l->len += k;
  l->buf[l->len] = '\0';
  l->col += utf8_width(s);
}

static void pad_to(Line *l, size_t col) {
  while (l->col < col && l->len + 1 < sizeof l->buf) put(l, " ");
}

static void put_right(Line *l, const char *s, size_t width) {
  size_t w = utf8_width(s);
  pad_to(l, l->col + (w < width ? width - w : 0));
  put(l, s);
}

/* ======================= status / labels ======================= */

static const char *disp_name(void) {
  static const char *n[] = { "FIX", "SCI", "ENG", "ALL" };
  return n[cfg.disp];
}

static const char *angle_name(void) {
  static const char *n[] = { "RAD", "DEG", "GRAD" };
  return n[cfg.angle];
}

static void print_status(const State *s) {
  char prec[16] = "";
  if (cfg.disp != DISP_ALL) snprintf(prec, sizeof prec, " %d", cfg.precision);
  printf("  [%s%s  %s  depth %d  R%d  undo %d]\n", disp_name(), prec, angle_name(),
         stack_depth(&s->stack), s->reg_idx, undo_depth());
}

static void level_label(char *buf, size_t n, int d) {
  if (d < 4) snprintf(buf, n, "%c:", "XYZT"[d]);
  else snprintf(buf, n, "%d:", d);
}

/* ======================= displays ======================= */

void print_stack_plain(const State *s) {
  char buf[64];
  for (int i = 0; i <= s->stack.top; i++) {
    fmt_num(buf, sizeof buf, s->stack.data[i]);
    printf("%s\n", buf);
  }
}

static void print_small(const State *s) {
  char lab[16], num[64];
  int depth = stack_depth(&s->stack);
  if (depth == 0) printf("  (empty)\n");
  for (int d = depth - 1; d >= 0; d--) {
    level_label(lab, sizeof lab, d);
    fmt_num(num, sizeof num, peek(&s->stack, d));
    printf("%5s %s\n", lab, num);
  }
  print_status(s);
}

#define SIDE_ROWS 7

static void print_big(const State *s) {
  const Stack *st = &s->stack;
  int depth = stack_depth(st);
  SigmaOut o;
  sigma_derive(&s->sig, &o);

  const char *in_lab[SIDE_ROWS]  = { "N", "Σx", "Σx²", "Σy", "Σy²", "Σxy", "" };
  double      in_val[SIDE_ROWS]  = { o.n, o.sx, o.sx2, o.sy, o.sy2, o.sxy, 0 };
  const char *out_lab[SIDE_ROWS] = { "x̄", "sₓ", "ȳ", "sᵧ", "r", "a", "b" };
  double      out_val[SIDE_ROWS] = { o.mx, o.sdx, o.my, o.sdy, o.r, o.a, o.b };

  /* column width: widest number that will be shown */
  char num[64];
  size_t w = 10;
  for (int d = 0; d < depth; d++) {
    fmt_num(num, sizeof num, peek(st, d));
    if (utf8_width(num) > w) w = utf8_width(num);
  }
  double side[] = { s->last_x, s->last_y };
  for (int i = 0; i < 2; i++) {
    fmt_num(num, sizeof num, side[i]);
    if (utf8_width(num) > w) w = utf8_width(num);
  }
  if (cfg.show_stat_regs)
    for (int i = 0; i < SIDE_ROWS; i++) {
      fmt_num(num, sizeof num, in_val[i]);
      if (utf8_width(num) > w) w = utf8_width(num);
      fmt_num(num, sizeof num, out_val[i]);
      if (utf8_width(num) > w) w = utf8_width(num);
    }

  size_t c_stack = 0, c_last = c_stack + 6 + w + 4;
  size_t c_in = c_last + 6 + w + 4, c_out = c_in + 5 + w + 4;

  int rows = depth > 4 ? depth : 4;
  if (cfg.show_stat_regs && rows < SIDE_ROWS) rows = SIDE_ROWS;

  for (int r = 0; r < rows; r++) {
    Line l = { .len = 0, .col = 0 };
    l.buf[0] = '\0';
    int d = rows - 1 - r;                 /* depth from top; X is the last row */
    char lab[16];
    if (d < depth || d < 4) {
      level_label(lab, sizeof lab, d);
      put_right(&l, lab, 5);
      put(&l, " ");
      if (d < depth) {
        fmt_num(num, sizeof num, peek(st, d));
        put_right(&l, num, w);
      }
    }
    if (r < 2) {
      pad_to(&l, c_last);
      put(&l, r == 0 ? "lstx " : "lsty ");
      fmt_num(num, sizeof num, side[r]);
      put_right(&l, num, w);
    }
    if (cfg.show_stat_regs && r < SIDE_ROWS) {
      if (in_lab[r][0]) {
        pad_to(&l, c_in);
        put(&l, in_lab[r]);
        pad_to(&l, c_in + 5);
        fmt_num(num, sizeof num, in_val[r]);
        put_right(&l, num, w);
      }
      pad_to(&l, c_out);
      put(&l, out_lab[r]);
      pad_to(&l, c_out + 3);
      fmt_num(num, sizeof num, out_val[r]);
      put_right(&l, num, w);
    }
    while (l.len > 0 && l.buf[l.len - 1] == ' ') l.buf[--l.len] = '\0';
    printf("%s\n", l.buf);
  }
  print_status(s);
}

void print_stack(const State *s) {
  if (cfg.small_terminal) print_small(s);
  else print_big(s);
}
