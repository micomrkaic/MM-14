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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "state.h"

Config cfg = {
  .precision = 6,
  .disp = DISP_FIX,
  .angle = ANG_RAD,
  .show_stat_regs = true,
  .small_terminal = false,
  .interactive = false,
  .quit = false,
};

volatile sig_atomic_t interrupted = 0;
int error_count = 0;

void state_init(State *s) {
  memset(s, 0, sizeof *s);
  s->stack.top = -1;
}

bool state_equal(const State *a, const State *b) {
  if (a->stack.top != b->stack.top) return false;
  for (int i = 0; i <= a->stack.top; i++)
    if (a->stack.data[i] != b->stack.data[i]) return false;
  if (a->last_x != b->last_x || a->last_y != b->last_y) return false;
  if (a->reg_idx != b->reg_idx) return false;
  for (int i = 0; i < MAX_REGS; i++)
    if (a->reg_used[i] != b->reg_used[i] || a->reg[i] != b->reg[i]) return false;
  return memcmp(&a->sig, &b->sig, sizeof a->sig) == 0;
}

/* ---- undo ring ---- */
static State ring[UNDO_LEVELS];
static int ring_head = 0;   /* next slot to write */
static int ring_count = 0;

void undo_push(const State *s) {
  ring[ring_head] = *s;
  ring_head = (ring_head + 1) % UNDO_LEVELS;
  if (ring_count < UNDO_LEVELS) ring_count++;
}

bool undo_pop(State *s) {
  if (ring_count == 0) return false;
  ring_head = (ring_head + UNDO_LEVELS - 1) % UNDO_LEVELS;
  ring_count--;
  *s = ring[ring_head];
  return true;
}

int undo_depth(void) { return ring_count; }

/* ---- messages ---- */
int rpn_err(const char *fmt, ...) {
  va_list ap;
  fflush(stdout);
  fputs("Error: ", stderr);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  error_count++;
  return RPN_ERR;
}

void rpn_info(const char *fmt, ...) {
  if (!cfg.interactive) return;
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  putchar('\n');
}
