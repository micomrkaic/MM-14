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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp.h"
#include "ops.h"
#include "stack.h"
#include "tokenizer.h"
#include "util_func.h"
#include "words.h"

void interp_init(Interp *in) {
  memset(in, 0, sizeof *in);
  in->mode = IN_NORMAL;
}

static void def_reset(Interp *in) {
  for (int i = 0; i < in->ntok; i++) free(in->toks[i]);
  in->ntok = 0;
  in->mode = IN_NORMAL;
  in->name[0] = '\0';
}

void interp_free(Interp *in) {
  def_reset(in);
  free(in->toks);
  in->toks = NULL;
  in->cap = 0;
}

bool interp_pending(const Interp *in) {
  return in->mode != IN_NORMAL || in->in_comment;
}

static int def_add(Interp *in, const char *tok) {
  if (in->ntok == in->cap) {
    int nc = in->cap ? 2 * in->cap : 32;
    char **nt = realloc(in->toks, (size_t)nc * sizeof *nt);
    if (!nt) return rpn_err("Out of memory");
    in->toks = nt;
    in->cap = nc;
  }
  if (!(in->toks[in->ntok] = strdup(tok))) return rpn_err("Out of memory");
  in->ntok++;
  return RPN_OK;
}

/* Execute one top-level token (number, built-in or user word). */
static int exec_token(State *s, const char *tok) {
  double v;
  switch (parse_number(tok, &v)) {
  case NUM_OK:    return push(&s->stack, v);
  case NUM_RANGE: return rpn_err("Number out of range: %s", tok);
  case NUM_NO:    break;
  }
  int op = find_op(tok);
  if (op >= 0) return call_op(s, op);
  int w = find_word(tok);
  if (w >= 0) return exec_word(s, w, 0);
  return rpn_err("Unknown word: %s", tok);
}

/* Atomic execution with rollback on error and an undo snapshot on change. */
static State before;                   /* static: ~3 KB, not re-entrant (no need) */

static int finish_atomic(State *s, int rc) {
  if (rc != RPN_OK) { *s = before; return RPN_ERR; }
  if (!state_equal(&before, s)) undo_push(&before);
  return RPN_OK;
}

static int run_atomic(State *s, const char *tok) {
  before = *s;
  return finish_atomic(s, exec_token(s, tok));
}

static int run_atomic_push(State *s, double v) {
  before = *s;
  return finish_atomic(s, push(&s->stack, v));
}

/* Gather tok and every remaining token of the line (minus comments), compile
   them as an anonymous word and run it. */
static int run_atomic_rest(Interp *in, State *s, char *tok, Tokenizer *t) {
  in->ntok = 0;
  int rc = def_add(in, tok);
  char *x;
  while (rc == RPN_OK && (x = next_token(t))) {
    size_t len = strlen(x);
    if (in->in_comment) { if (x[len - 1] == ')') in->in_comment = false; continue; }
    if (x[0] == '(') { if (!(len > 1 && x[len - 1] == ')')) in->in_comment = true; continue; }
    if (strcmp(x, "\\") == 0) break;
    rc = def_add(in, x);
  }
  if (rc == RPN_OK) {
    before = *s;
    rc = finish_atomic(s, exec_tokens(s, in->toks, in->ntok));
  }
  for (int i = 0; i < in->ntok; i++) free(in->toks[i]);
  in->ntok = 0;
  in->in_comment = false;
  return rc;
}

int interp_line(Interp *in, State *s, char *line) {
  Tokenizer t = make_tokenizer(line, WHITESPACE);
  char *tok;
  bool see_pending = false;
  int rc = RPN_OK;

  while ((tok = next_token(&t)) && !cfg.quit) {
    size_t len = strlen(tok);

    /* comments */
    if (in->in_comment) {
      if (tok[len - 1] == ')') in->in_comment = false;
      continue;
    }
    if (tok[0] == '(' ) {
      if (!(len > 1 && tok[len - 1] == ')')) in->in_comment = true;
      continue;
    }
    if (strcmp(tok, "\\") == 0) break;

    /* definitions */
    if (in->mode == IN_DEF_NAME) {
      if (strcmp(tok, ";") == 0 || strcmp(tok, ":") == 0) {
        rc = rpn_err("Missing word name after ':'");
        def_reset(in);
        break;
      }
      if (strlen(tok) >= MAX_WORD_NAME) {
        rc = rpn_err("Word name longer than %d characters", MAX_WORD_NAME - 1);
        def_reset(in);
        break;
      }
      snprintf(in->name, sizeof in->name, "%s", tok);
      in->mode = IN_DEF_BODY;
      continue;
    }
    if (in->mode == IN_DEF_BODY) {
      if (strcmp(tok, ";") == 0) {
        rc = define_word(in->name, in->toks, in->ntok);
        def_reset(in);
        if (rc != RPN_OK) break;
      } else if (strcmp(tok, ":") == 0) {
        rc = rpn_err("Nested ':' inside the definition of '%s'", in->name);
        def_reset(in);
        break;
      } else if (def_add(in, tok) != RPN_OK) {
        rc = RPN_ERR;
        def_reset(in);
        break;
      }
      continue;
    }

    /* top level */
    if (see_pending) {
      see_pending = false;
      if ((rc = see_word(tok)) != RPN_OK) break;
      continue;
    }
    if (strcmp(tok, ":") == 0) { in->mode = IN_DEF_NAME; continue; }
    if (strcmp(tok, ";") == 0) { rc = rpn_err("';' without ':'"); break; }
    if (strcmp(tok, "see") == 0) { see_pending = true; continue; }
    if (strcmp(tok, "u") == 0) {
      if (!undo_pop(s)) { rc = rpn_err("Nothing to undo"); break; }
      continue;
    }
    if (strcmp(tok, "'") == 0) {
      char *nm = next_token(&t);
      if (!nm) { rc = rpn_err("' needs a word name after it"); break; }
      double xt = xt_of(nm);
      if (xt < 0) { rc = rpn_err("' of undefined word '%s'", nm); break; }
      if ((rc = run_atomic_push(s, xt)) != RPN_OK) break;
      continue;
    }
    if (is_control_word(tok)) {
      /* Control flow outside a definition: compile the rest of the line as
         one anonymous word and run it atomically. */
      if ((rc = run_atomic_rest(in, s, tok, &t)) != RPN_OK) break;
      continue;
    }
    if ((rc = run_atomic(s, tok)) != RPN_OK) break;
  }

  if (see_pending && rc == RPN_OK) rc = rpn_err("see: missing word name");
  if (rc != RPN_OK) in->in_comment = false;
  return rc;
}

int interp_finish(Interp *in) {
  int rc = RPN_OK;
  if (in->mode != IN_NORMAL)
    rc = rpn_err("Unterminated definition%s%s", in->name[0] ? " of " : "", in->name);
  def_reset(in);
  in->in_comment = false;
  return rc;
}
