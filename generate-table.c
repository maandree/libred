/* See LICENSE file for copyright and license details. */
#include "libred.h"
#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#if __GNUC__
# pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

struct xy {double x, y;};

static struct xy xy_table[] = {
#include "10deg-xy.i"
};

/* define ciexyy_to_srgb() and adjust_luma() */
#define LIBRED_COMPILING_PARSER
#include "blackbody.c"

int
main(void)
{
	long int temp = LIBRED_LOWEST_TEMPERATURE;
	size_t i, n = sizeof(xy_table) / sizeof(*xy_table);
	double r, g, b;

	for (i = 0; i < n; i++, temp += LIBRED_DELTA_TEMPERATURE) {
		if (temp == 6500)
			r = g = b = 1;
		else
			ciexyy_to_srgb(xy_table[i].x, xy_table[i].y, 1, &r, &g, &b);
		printf("{%a, %a, %a}%s\n", r, g, b, i + 1 == n ? "" : ",");
	}

	if (fflush(stdout) || ferror(stdout) || fclose(stdout)) {
		perror(NULL);
		return -1;
	}

	return 0;
}
