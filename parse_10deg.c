/* See LICENSE file for copyright and license details. */
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
int
main(int argc, char *argv[])
{
#define x  (xyrgb[0])
#define y  (xyrgb[1])
#define r  (xyrgb[2])
#define g  (xyrgb[3])
#define b  (xyrgb[4])
  
	double xyrgb[5];
	struct stat attr;
	long int temp = LIBRED_LOWEST_TEMPERATURE;
  
	for (; fscanf(stdin, "%lf %lf\n", &x, &y) == 2; temp += LIBRED_DELTA_TEMPERATURE) {
		(temp == 6500) ? (r = g = b = 1.0) : ciexyy_to_srgb(x, y, 1.0, &r, &g, &b);
		xwrite(STDOUT_FILENO, xyrgb, sizeof(xyrgb));
	}
	xwrite(STDOUT_FILENO, xyrgb, sizeof(xyrgb)); /* sugar */
	t (fstat(STDOUT_FILENO, &attr));
	return ((size_t)(attr.st_size) != (TEMPERATURES + 1) * 5 * sizeof(double));
fail:
	return perror(argc ? *argv : "parse_10deg"), 1;
}
