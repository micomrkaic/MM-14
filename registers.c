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
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "registers.h"
#include "stack.h"
#include "stack_printing.h"
#include "util_func.h"

#define REG_FILE "registers.txt"

int reg_select(State *s) {
  double v = pop(&s->stack);
  if (v != floor(v) || v < 0 || v >= MAX_REGS)
    return rpn_err("regidx: register index must be an integer 0..%d", MAX_REGS - 1);
  s->reg_idx = (int)v;
  return RPN_OK;
}

int reg_store(State *s, RegOp op) {
  int i = s->reg_idx;
  double x = peek(&s->stack, 0);
  double r = s->reg[i];
  switch (op) {
  case REG_STO: r = x; break;
  case REG_ADD: r += x; break;
  case REG_SUB: r -= x; break;
  case REG_MUL: r *= x; break;
  case REG_DIV:
    if (x == 0.0) return rpn_err("sto/: division by zero");
    r /= x;
    break;
  }
  if (!isfinite(r)) return rpn_err("Register %d would overflow", i);
  s->reg[i] = r;
  s->reg_used[i] = true;
  char buf[64];
  fmt_num(buf, sizeof buf, r);
  rpn_info("R%d = %s", i, buf);
  return RPN_OK;
}

int reg_recall(State *s) {
  int i = s->reg_idx;
  if (!s->reg_used[i]) return rpn_err("Register %d is empty", i);
  return push(&s->stack, s->reg[i]);
}

void reg_clear(State *s) {
  memset(s->reg, 0, sizeof s->reg);
  memset(s->reg_used, 0, sizeof s->reg_used);
}

void reg_list(const State *s) {
  bool any = false;
  char buf[64];
  for (int i = 0; i < MAX_REGS; i++) {
    if (!s->reg_used[i]) continue;
    fmt_num(buf, sizeof buf, s->reg[i]);
    printf("  R%-2d %c %s\n", i, i == s->reg_idx ? '*' : ' ', buf);
    any = true;
  }
  if (!any) printf("  (no registers in use; current index %d)\n", s->reg_idx);
}

int reg_save(const State *s) {
  char path[1200], tmp[1210];
  TRY(config_path(REG_FILE, path, sizeof path, true));
  snprintf(tmp, sizeof tmp, "%s.tmp", path);
  FILE *f = fopen(tmp, "w");
  if (!f) return rpn_err("Cannot write %s: %s", tmp, strerror(errno));
  for (int i = 0; i < MAX_REGS; i++)
    if (s->reg_used[i]) fprintf(f, "%d %.17g\n", i, s->reg[i]);
  if (fclose(f) != 0 || rename(tmp, path) != 0)
    return rpn_err("Cannot save %s: %s", path, strerror(errno));
  rpn_info("Registers saved to %s", path);
  return RPN_OK;
}

int reg_load(State *s, bool quiet_if_missing) {
  char path[1200];
  TRY(config_path(REG_FILE, path, sizeof path, false));
  FILE *f = fopen(path, "r");
  if (!f) {
    if (quiet_if_missing && errno == ENOENT) return RPN_OK;
    return rpn_err("Cannot read %s: %s", path, strerror(errno));
  }
  reg_clear(s);
  char line[256];
  int lineno = 0, rc = RPN_OK;
  while (fgets(line, sizeof line, f)) {
    lineno++;
    int r;
    char num[128];
    double v;
    if (line[0] == '#' || strspn(line, " \t\r\n") == strlen(line)) continue;
    if (sscanf(line, "%d %127s", &r, num) != 2 || r < 0 || r >= MAX_REGS ||
        parse_number(num, &v) != NUM_OK) {
      rc = rpn_err("%s:%d: bad register line", path, lineno);
      continue;
    }
    s->reg[r] = v;
    s->reg_used[r] = true;
  }
  fclose(f);
  return rc;
}
