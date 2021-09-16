/* See LICENSE file for copyright and license details. */
#if defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunsuffixed-float-constants"
#endif

#ifndef LIBRED_COMPILING_PARSER
# include "libred.h"
# include <errno.h>
# include <math.h>
# include <stddef.h>


/**
 * Colour temperatures in CIE xy (xyY without Y)
 */
static struct xy {double x, y;} xy_table[] = {
# include "10deg-xy.i"
};

/**
 * Colour temperatures in sRGB
 */
static struct rgb {double r, g, b;} rgb_table[] = {
# include "10deg-rgb.i"
};


#endif
/**
 * Convert from CIE xyY to [0, 1] sRGB
 * 
 * @param  x  The 'x' component
 * @param  y  The 'y' component
 * @param  Y  The 'Y' component
 * @param  r  Output parameter for the “red” value
 *            (Seriously, sRGB red is orange, just look at it fullscreen)
 * @param  g  Output parameter for the green value
 * @param  b  Output parameter for the blue value
 */
static void
ciexyy_to_srgb(double x, double y, double Y, double *r, double *g, double *b)
{
#define SRGB(C)  (((C) <= 0.0031308) ? (12.92 * (C)) : ((1.0 + 0.055) * pow((C), 1.0 / 2.4) - 0.055))
	double X, Z, max;
  
#if __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
	/* Convert CIE xyY to CIE XYZ */
	X = Y * (y == 0.0 ? 0.0 : (x / y));
	Z = Y * (y == 0.0 ? 0.0 : ((1.0 - x - y) / y));
#if __GNUC__
# pragma GCC diagnostic pop
#endif
  
	/* Convert CIE XYZ to [0, 1] linear RGB (ciexyz_to_linear) */
	*r = ( 3.240450 * X) + (-1.537140 * Y) + (-0.4985320 * Z);
	*g = (-0.969266 * X) + ( 1.876010 * Y) + ( 0.0415561 * Z);
	*b = (0.0556434 * X) + (-0.204026 * Y) + ( 1.0572300 * Z);
  
	/* Convert [0, 1] linear RGB to [0, 1] sRGB */
	SRGB(*r), SRGB(*g), SRGB(*b);

	/* Adjust colours for use */
	max = fmax(fmax(fabs(*r), fabs(*g)), fabs(*b));
#if __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
	if (max != 0.0)  *r /= max, *g /= max, *b /= max;
#if __GNUC__
# pragma GCC diagnostic pop
#endif
	*r = *r > 0.0 ? *r : 0.0;
	*g = *g > 0.0 ? *g : 0.0;
	*b = *b > 0.0 ? *b : 0.0;
}
#ifndef LIBRED_COMPILING_PARSER

/**
 * Perform linear interpolation (considered very good)
 * between the CIE xyY values for two colour temperatures
 * and convert the result to sRGB. The two colours should
 * be the closest below the desired colour temperature,
 * and the closest above the desired colour temperature
 * 
 * @param  x1    The 'x' component for the low colour
 * @param  y1    The 'y' component for the low colour
 * @param  x2    The 'x' component for the high colour
 * @param  y2    The 'y' component for the high colour
 * @param  temp  The desired colour temperature
 * @param  r     Output parameter for the “red” value
 * @param  g     Output parameter for the green value
 * @param  b     Output parameter for the blue value
 */
static void
interpolate(double x1, double y1, double x2, double y2, double temp, double *r, double *g, double *b)
{
	double weight = fmod(temp - (LIBRED_LOWEST_TEMPERATURE % LIBRED_DELTA_TEMPERATURE),
	                     (double)LIBRED_DELTA_TEMPERATURE) / (double)LIBRED_DELTA_TEMPERATURE;
	double x = x1 * (1 - weight) + x2 * weight;
	double y = y1 * (1 - weight) + y2 * weight;
	ciexyy_to_srgb(x, y, 1.0, r, g, b);
}

/**
 * Get the [0, 1] sRGB values of a colour temperature
 * 
 * libred has a table of colour temperature values, this
 * function interpolates values that are missing. If you
 * don't want any interpolation the `temp` parameter can
 * be specified in one of the following ways:
 * 
 * - floor:
 *         (temp - LIBRED_LOWEST_TEMPERATURE) /
 *         LIBRED_DELTA_TEMPERATURE *
 *         LIBRED_DELTA_TEMPERATURE +
 *         LIBRED_LOWEST_TEMPERATURE
 * 
 * - ceiling:
 *         (temp - LIBRED_LOWEST_TEMPERATURE +
 *          LIBRED_DELTA_TEMPERATURE - 1) /
 *         LIBRED_DELTA_TEMPERATURE *
 *         LIBRED_DELTA_TEMPERATURE +
 *         LIBRED_LOWEST_TEMPERATURE
 * 
 * - round to nearest:
 *         (temp - LIBRED_LOWEST_TEMPERATURE +
 *          LIBRED_DELTA_TEMPERATURE / 2) /
 *         LIBRED_DELTA_TEMPERATURE *
 *         LIBRED_DELTA_TEMPERATURE +
 *         LIBRED_LOWEST_TEMPERATURE
 * 
 * @param   temp  The desired colour temperature
 * @param   r     Output parameter for the “red” value
 * @param   g     Output parameter for the green value
 * @param   b     Output parameter for the blue value
 * @return        0 on succeess, -1 on error
 * @throws  EDOM  The selected temperature is below 1000K
 */
int
libred_get_colour(long int temp, double *r, double *g, double *b)
{
	double x1, y1, x2, y2;
	size_t i;
	long int tmp;

	if (temp > LIBRED_HIGHEST_TEMPERATURE)
		temp = LIBRED_HIGHEST_TEMPERATURE;

	if (temp < LIBRED_LOWEST_TEMPERATURE) {
		errno = EDOM;
		return -1;
	}

	tmp = temp - LIBRED_LOWEST_TEMPERATURE;
	
	i = (size_t)(tmp / LIBRED_DELTA_TEMPERATURE);
	if (tmp % LIBRED_DELTA_TEMPERATURE) {
		x1 = xy_table[i].x;
		y1 = xy_table[i].y;
		x2 = xy_table[i + 1].x;
		y2 = xy_table[i + 1].y;
		interpolate(x1, y1, x2, y2, (double)temp, r, g, b);
	} else {
		*r = rgb_table[i].r;
		*g = rgb_table[i].g;
		*b = rgb_table[i].b;
	}

	return 0;
}
#endif


#if __GNUC__
# pragma GCC diagnostic pop
#endif
