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
#include <math.h>
#include <sys/stat.h>



/**
 * The number of measured temperatures.
 */
#define TEMPERATURES  ((LIBRED_HIGHEST_TEMPERATURE - LIBRED_LOWEST_TEMPERATURE) / LIBRED_DELTA_TEMPERATURE + 1)



/**
 * Convert from CIE xyY to [0, 1] sRGB.
 * 
 * This function is identical to that in 'blackbody.c'.
 * 
 * @param  x  The 'x' component.
 * @param  y  The 'y' component.
 * @param  Y  The 'Y' component.
 * @param  r  Output parameter for the “red” value.
 *            (Seriously, sRGB red is orange, just look at it fullscreen.)
 * @param  g  Output parameter for the green value.
 * @param  b  Output parameter for the blue value.
 */
static void ciexyy_to_srgb(double x, double y, double Y, double *r, double *g, double *b)
{
#define SRGB(C)  (((C) <= 0.0031308) ? (12.92 * (C)) : ((1.0 + 0.055) * pow((C), 1.0 / 2.4) - 0.055))
  double X, Z;
  
  /* Convert CIE xyY to CIE XYZ. */
  X = Y * (y == 0.0 ? 0.0 : (x / y));
  Z = Y * (y == 0.0 ? 0.0 : ((1.0 - x - y) / y));
  
  /* Convert CIE XYZ to [0, 1] linear RGB. (ciexyz_to_linear) */
  *r = ( 3.240450 * X) + (-1.537140 * Y) + (-0.4985320 * Z);
  *g = (-0.969266 * X) + ( 1.876010 * Y) + ( 0.0415561 * Z);
  *b = (0.0556434 * X) + (-0.204026 * Y) + ( 1.0572300 * Z);
  
  /* Convert [0, 1] linear RGB to [0, 1] sRGB. */
  SRGB(*r), SRGB(*g), SRGB(*b);
}


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

