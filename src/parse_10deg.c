/**
 * Copyright © 2016  Mattias Andrée <maandree@member.fsf.org>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "libred.h"
#include "macros.h"
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>



/**
 * The number of measured temperatures.
 */
#define TEMPERATURES  ((LIBRED_HIGHEST_TEMPERATURE - LIBRED_LOWEST_TEMPERATURE) / LIBRED_DELTA_TEMPERATURE + 1)



#define LIBRED_COMPILING_PARSER
#include "blackbody.c"



/**
 * Create the lookup table of temperatures.
 * 
 * Standard input should be the file '10deg',
 * standard output should be the table file
 * and must be a regular file.
 * 
 * @param   argc  Should be 1.
 * @param   argv  Should only contain the name of the process.
 * @return        0 on success, 1 on error.
 */
int main(int argc, char *argv[])
{
#define x  (xyrgb[0])
#define y  (xyrgb[1])
#define r  (xyrgb[2])
#define g  (xyrgb[3])
#define b  (xyrgb[4])
  
  double xyrgb[5];
  struct stat attr;
  long int temp = LIBRED_LOWEST_TEMPERATURE;
  
  for (; fscanf(stdin, "%lf %lf\n", &x, &y) == 2; temp += LIBRED_DELTA_TEMPERATURE)
    {
      (temp == 6500) ? (r = g = b = 1.0) : ciexyy_to_srgb(x, y, 1.0, &r, &g, &b);
      xwrite(STDOUT_FILENO, xyrgb, sizeof(xyrgb));
    }
  xwrite(STDOUT_FILENO, xyrgb, sizeof(xyrgb)); /* sugar */
  t (fstat(STDOUT_FILENO, &attr));
  return ((size_t)(attr.st_size) != (TEMPERATURES + 1) * 5 * sizeof(double));
 fail:
  return perror(argc ? *argv : "parse_10deg"), 1;
}

