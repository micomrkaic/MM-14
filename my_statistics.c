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
#include <math.h>
#include "globals.h"

// Functions for statistical registers
void sigma_clear(void)
{
  for (int i = 0; i < 6; i++) {
    Reg_stat_in[i] = 0.0;
    Reg_stat_out[i] = 0.0;
  }
}

double sigma_plus(double x, double y) {
  Reg_stat_in[1]+=x;
  Reg_stat_in[2]+=x*x;
  Reg_stat_in[3]+=y;
  Reg_stat_in[4]+=y*y;
  Reg_stat_in[5]+=x*y;
  return Reg_stat_in[0]+=1;
}

double sigma_minus(double x, double y) {
  if (Reg_stat_in[0] > 0) {
    Reg_stat_in[1]-=x;
    Reg_stat_in[2]-=x*x;
    Reg_stat_in[3]-=y;
    Reg_stat_in[4]-=y*y;
    Reg_stat_in[5]-=x*y;
    Reg_stat_in[0]--;
    return Reg_stat_in[0]-=1;
  } else return 0.0;
}

// Print the statistics
void sigma_evaluate(void) {
  if (Reg_stat_in[0] > 0) {
    double n = Reg_stat_in[0];
    double x_bar = Reg_stat_in[1]/n;
    double y_bar = Reg_stat_in[3]/n;
    double xy_bar = Reg_stat_in[5]/n;
    double x2_bar = Reg_stat_in[2]/n;
    double y2_bar = Reg_stat_in[4]/n;

    double std_x=sqrt(x2_bar-x_bar*x_bar);
    double std_y=sqrt(y2_bar-y_bar*y_bar);
    double rho = (xy_bar-x_bar*y_bar)/(std_x*std_y);

    Reg_stat_out[0]=n;
    Reg_stat_out[1]=x_bar;
    Reg_stat_out[2]=y_bar;
    Reg_stat_out[3]=std_x;
    Reg_stat_out[4]=std_y;
    Reg_stat_out[5]=rho;
    
  } else printf("No entries in statistical registers!\n");
}

