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
/* Mico's simple RPN Calculator -- MM-14

   Still to do (for later calculators)
   MM-15: complex numbers, matrices, numerical integration and zero finding
   MM-48: terminal plotting, symbolic algebra, strings, lists, units
*/

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "state.h"
#include "interp.h"
#include "my_math.h"
#include "registers.h"
#include "words.h"
#include "stack_printing.h"
#include "help_functions.h"
#include "tab_complete.h"
#include "util_func.h"

#define HISTORY_LINES 1000

static void on_sigint(int sig) {
  (void)sig;
  interrupted = 1;
}

static void install_sigint(void) {
  struct sigaction sa;
  memset(&sa, 0, sizeof sa);
  sa.sa_handler = on_sigint;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
}

static void usage(FILE *f) {
  fprintf(f,
    "usage: mm_14 [-n] [-e EXPR]... [-f FILE]...\n"
    "       mm_14 --commands | --version | -h\n"
    "\n"
    "  (no args)   interactive calculator (reads stdin when it is not a terminal)\n"
    "  -e EXPR     evaluate EXPR; may be repeated\n"
    "  -f FILE     run the lines of FILE; may be repeated\n"
    "  -n          do not autoload saved words and registers\n"
    "  --commands  print the command reference as Markdown\n"
    "\n"
    "Non-interactive runs print the final stack, one value per line, and exit\n"
    "with status 1 if any error occurred.\n");
}

static void run_line(Interp *in, State *s, const char *text) {
  char *copy = strdup(text);
  if (!copy) { rpn_err("Out of memory"); return; }
  interrupted = 0;
  interp_line(in, s, copy);
  free(copy);
}

static int run_file(Interp *in, State *s, FILE *f) {
  char *line = NULL;
  size_t cap = 0;
  while (!cfg.quit && getline(&line, &cap, f) != -1) {
    interrupted = 0;
    interp_line(in, s, line);
  }
  free(line);
  return RPN_OK;
}

static void history_path(char *out, size_t n) {
  if (config_path("history", out, n, true) != RPN_OK) out[0] = '\0';
}

static void interactive(Interp *in, State *s) {
  char hist[1200];
  int errs_before = error_count;
  history_path(hist, sizeof hist);
  error_count = errs_before;          /* a missing config dir is not worth an error */
  stifle_history(HISTORY_LINES);
  if (hist[0]) read_history(hist);
  completion_init();

  splash_screen();
  print_stack(s);

  char *input;
  while (!cfg.quit && (input = readline(interp_pending(in) ? "... " : "> "))) {
    if (*input) {
      add_history(input);
      interrupted = 0;
      interp_line(in, s, input);
    }
    free(input);
    if (!interp_pending(in) && !cfg.quit) print_stack(s);
  }
  if (!cfg.quit) putchar('\n');     /* Ctrl-D leaves the cursor on the prompt line */
  interp_finish(in);
  if (hist[0]) write_history(hist);
  printf("Goodbye!\n");
}

int main(int argc, char **argv) {
  enum { MAXJOBS = 128 };
  struct { char kind; const char *arg; } jobs[MAXJOBS];
  int njobs = 0;
  bool autoload = true;

  for (int i = 1; i < argc; i++) {
    const char *a = argv[i];
    if (!strcmp(a, "-h") || !strcmp(a, "--help")) { usage(stdout); return 0; }
    if (!strcmp(a, "--version")) { printf("MM-14 %s\n", MM14_VERSION); return 0; }
    if (!strcmp(a, "--commands")) { print_commands_markdown(); return 0; }
    if (!strcmp(a, "-n")) { autoload = false; continue; }
    if ((!strcmp(a, "-e") || !strcmp(a, "-f")) && i + 1 < argc && njobs < MAXJOBS) {
      jobs[njobs].kind = a[1];
      jobs[njobs].arg = argv[++i];
      njobs++;
      continue;
    }
    usage(stderr);
    return 2;
  }

  State s;
  state_init(&s);
  Interp in;
  interp_init(&in);
  rng_seed_u64((unsigned long long)time(NULL) ^ ((unsigned long long)getpid() << 32));
  install_sigint();

  cfg.interactive = (njobs == 0 && isatty(STDIN_FILENO));

  if (autoload) {
    load_words(true);
    reg_load(&s, true);
  }

  if (cfg.interactive) {
    interactive(&in, &s);
  } else {
    if (njobs == 0) {
      run_file(&in, &s, stdin);
    } else {
      for (int j = 0; j < njobs && !cfg.quit; j++) {
        if (jobs[j].kind == 'e') {
          run_line(&in, &s, jobs[j].arg);
        } else {
          FILE *f = strcmp(jobs[j].arg, "-") ? fopen(jobs[j].arg, "r") : stdin;
          if (!f) { rpn_err("Cannot open %s: %s", jobs[j].arg, strerror(errno)); continue; }
          run_file(&in, &s, f);
          if (f != stdin) fclose(f);
        }
      }
    }
    interp_finish(&in);
    print_stack_plain(&s);
  }

  interp_free(&in);
  free_words();
  return (!cfg.interactive && error_count) ? 1 : 0;
}
