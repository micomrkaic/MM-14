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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "stack_printing.h"
#include "stack.h"
#include "my_statistics.h"

void printStack_small_terminal(Stack* s, int precision, char* stack_title) {
  printf(" **** %s **** \n", stack_title);
  if (isEmpty(s)) {
    printf("{}\n");
    return;
  }
  char format[32];
  if (fixed_point_format)
    sprintf(format, "[%%d] %%*.*f\n");
  else
    sprintf(format, "[%%d] %%*.*e\n");
   
  for (int i = 0; i <= s->top; i++) {
    printf(format, i, precision + 3, precision, s->data[i]);
  }
  printf("\n");
}

void printStack_big_terminal(Stack* s, int precision) {
  bool verbose = true;
  char line[TERM_WIDTH + 1];
  char num_buf[64];
  int max_rows = s->top + 1;
  if (verbose && max_rows < 6) max_rows = 6;

  for (int i = 0; i < max_rows; i++) {
    memset(line, ' ', TERM_WIDTH);
    line[TERM_WIDTH] = '\0';

    if (i <= s->top) {
      if (fixed_point_format)
	snprintf(num_buf, sizeof(num_buf), "[%2d] %*.*f", i, precision + 4, precision, s->data[i]);
      else
	snprintf(num_buf, sizeof(num_buf), "[%2d] %*.*e", i, precision + 4, precision, s->data[i]);

      memcpy(&line[0], num_buf, strlen(num_buf));
    }

    if (verbose) {
      if (i == 0) {
	if (fixed_point_format)
	  snprintf(num_buf, sizeof(num_buf), "last_y: %*.*f", precision + 4, precision, last_y);
	else
	  snprintf(num_buf, sizeof(num_buf), "last_y: %*.*e", precision + 4, precision, last_y);
	memcpy(&line[20], num_buf, strlen(num_buf));
      } else if (i == 1) {
	if (fixed_point_format)
	  snprintf(num_buf, sizeof(num_buf), "last_x: %*.*f", precision + 4, precision, last_x);
	else
	  snprintf(num_buf, sizeof(num_buf), "last_x: %*.*e", precision + 4, precision, last_x);
	memcpy(&line[20], num_buf, strlen(num_buf));
      }

      if (i < 6 && show_stat_regs) {
	// Use symbolic names
	const char* label_in = stat_in_labels[i];
	const char* label_out = stat_out_labels[i];

	if (fixed_point_format) {
	  snprintf(num_buf, sizeof(num_buf), "%s: %*.*f", label_in, precision + 4, precision, Reg_stat_in[i]);
	  memcpy(&line[40], num_buf, strlen(num_buf));
	  snprintf(num_buf, sizeof(num_buf), "%s: %*.*f", label_out, precision + 4, precision, Reg_stat_out[i]);
	  memcpy(&line[65], num_buf, strlen(num_buf));
	} else {
	  snprintf(num_buf, sizeof(num_buf), "%s: %*.*e", label_in, precision + 4, precision, Reg_stat_in[i]);
	  memcpy(&line[40], num_buf, strlen(num_buf));
	  snprintf(num_buf, sizeof(num_buf), "%s: %*.*e", label_out, precision + 4, precision, Reg_stat_out[i]);
	  memcpy(&line[65], num_buf, strlen(num_buf));
	}
      }
    }
    printf("%s\n", line);
  }
}

void printStack(Stack* stack, int precision, char* stack_title) {
  if (small_terminal)
    printStack_small_terminal(stack, precision,stack_title);
  else
    printStack_big_terminal(stack, precision);
}
