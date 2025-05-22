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
#include <time.h>

void help_screen(void) {
  printf("Type help for help; q or ctrl-d to quit. All input is case sensitive.\n");
  printf("Basic principles\n");
  printf("    The calculator is stack based and operates using RPN.\n");
  printf("    Sole limited programmability is possible.\n");
  printf("Math functionality\n");
  printf("    number followed by Enter pushes a number onto the stack.\n");
  printf("    + - * / abs inv chs %% %%chg frac int\n");
  printf("    sin cos tan asin acos atan sinh cosh tanh asinh acosh atanh\n");
  printf("    ln log exp ^ pow integrate fzero\n");
  printf("Sample statistics and basic probability\n");
  printf("    s+ s- clstat getstats npdf ncdf nqnt\n");
  printf("Stack operations\n");
  printf("    clst dup drop swap nip tuck roll lstx lsty stacksize stacksum stacksumsq\n");
  printf("Tests and logic\n");
  printf("    == <= >= != < > && || !\n");
  printf("Register operations\n");
  printf("    sto rcl sto+ sto- sto* sto regidx listregs clrg loadregs saveregs\n");
  printf("Uder defined words\n");
  printf("    listwords clearwords delword savewords loadwords\n");
  printf("Program controls\n");
  printf("    u [undo] p_prec {1-9} [print precision]\n");
  printf("    int_prec [integration precision] internals\n");
  printf("    f0_prec [fsolve precision] word_sel [user word selection]\n");
  printf("    sss [stats on<->off] spf [SCI<->ENG]\n");
  printf("\n");
}

// ANSI color codes
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"

void splash_screen(void) {
    // Get current time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    // Print splash screen
    printf(BOLD CYAN "\n===========================================\n" RESET);
    printf(BOLD GREEN "   MM-14 RPN CALCULATOR\n" RESET);
    printf(YELLOW "   Reverse Polish notation. Extensible.\n" RESET);
    printf(CYAN   "   Some math. Integration. Equation solving. \n" RESET);
    printf(MAGENTA "   Started at: %s\n" RESET, time_str);
    printf(BOLD CYAN "===========================================\n\n" RESET);
}
