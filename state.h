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
#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include <signal.h>

#define MM14_VERSION        "2.0"
#define MAX_STACK           256
#define MAX_REGS            100
#define UNDO_LEVELS         64
#define MAX_RECURSION_DEPTH 256
#define MAX_LOOP_NEST       16
#define MAX_WORD_NAME       32
#define MAX_PRECISION       15

#define RPN_OK   0
#define RPN_ERR  (-1)
#define TRY(e) do { int rc_ = (e); if (rc_ != RPN_OK) return rc_; } while (0)

typedef struct {
  double data[MAX_STACK];
  int top;                       /* -1 when empty */
} Stack;

/* Welford accumulators for bivariate statistics (exact s- support). */
typedef struct {
  double n, mx, my, m2x, m2y, cxy;
} Sigma;

/* Everything an operation can change. Snapshotted for undo and error rollback. */
typedef struct {
  Stack  stack;
  double last_x, last_y;
  double reg[MAX_REGS];
  bool   reg_used[MAX_REGS];
  int    reg_idx;
  Sigma  sig;
} State;

typedef enum { DISP_FIX, DISP_SCI, DISP_ENG, DISP_ALL } DispMode;
typedef enum { ANG_RAD, ANG_DEG, ANG_GRAD } AngleMode;

/* Settings: not part of undo. */
typedef struct {
  int       precision;
  DispMode  disp;
  AngleMode angle;
  bool      show_stat_regs;
  bool      small_terminal;
  bool      interactive;
  bool      quit;
} Config;

extern Config cfg;
extern volatile sig_atomic_t interrupted;
extern int error_count;

void state_init(State *s);
bool state_equal(const State *a, const State *b);

void undo_push(const State *s);
bool undo_pop(State *s);
int  undo_depth(void);

/* Print "Error: ..." to stderr, count it, return RPN_ERR. */
int  rpn_err(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
/* Confirmation messages: shown only in interactive mode. */
void rpn_info(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#endif
