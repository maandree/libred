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
 * between the CIE xy values for two colour temperatures
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
 * Perform linear interpolation (considered very good)
 * between the CIE xy values for two colour temperatures.
 * The two colours should be the closest below the desired
 * colour temperature, and the closest above the desired
 * colour temperature
 * 
 * @param  x1    The 'x' component for the low colour
 * @param  y1    The 'y' component for the low colour
 * @param  x2    The 'x' component for the high colour
 * @param  y2    The 'y' component for the high colour
 * @param  temp  The desired colour temperature
 * @param  x     Output parameter for the CIE x value
 * @param  y     Output parameter for the CIE y value
 */
static void
interpolate_xy(double x1, double y1, double x2, double y2, double temp, double *x, double *y)
{
	double weight = fmod(temp - (LIBRED_LOWEST_TEMPERATURE % LIBRED_DELTA_TEMPERATURE),
	                     (double)LIBRED_DELTA_TEMPERATURE) / (double)LIBRED_DELTA_TEMPERATURE;
	*x = x1 * (1 - weight) + x2 * weight;
	*y = y1 * (1 - weight) + y2 * weight;
}

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

int
libred_get_colour_xy(long int temp, double *x, double *y) /* TODO test */
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
		interpolate_xy(x1, y1, x2, y2, (double)temp, x, y);
	} else {
		*x = xy_table[i].x;
		*y = xy_table[i].y;
	}

	return 0;
}

double
libred_get_temperature_xy(double x, double y, double *x_error, double *y_error) /* TODO man, test */
{
	size_t i, j;
	double x1, y1, x2, y2, dx, dy, xd, yd, t, d2;
	double best_temp = 0, best_d2 = INFINITY;

	if (!x_error)
		x_error = &dx;
	if (!y_error)
		y_error = &dy;

	x1 = xy_table[0].x;
	y1 = xy_table[0].y;

	for (j = 1; j < sizeof(xy_table) / sizeof(*xy_table); j++) {
		x2 = xy_table[j].x;
		y2 = xy_table[j].y;
		dx = x2 - x1;
		dy = y2 - y1;

		t = ((x - x1) * dx + (y - y1) * dy) / (dx * dx + dy * dy);
		if (!isfinite(t))
			continue;
		t = t < 0 ? 0 : t;
		t = t > 1 ? 1 : t;

		xd = dx * t + x1 - x;
		yd = dy * t + y1 - y;
		d2 = xd * xd + yd * yd;

		if (d2 < best_d2) {
			*x_error = xd;
			*y_error = yd;
			best_d2 = d2;
			t *= (double)(j - i);
			best_temp = (double)i + t;
		}
	}

	return best_temp * LIBRED_DELTA_TEMPERATURE + LIBRED_LOWEST_TEMPERATURE;
}

/**
 * Convert from [0, 1] sRGB to CIE xyY
 * 
 * @param  r  The “red” value
 *            (Seriously, sRGB red is orange, just look at it fullscreen)
 * @param  g  The green value
 * @param  b  The blue value
 * @param  x  Output parameter for The 'x' component
 * @param  y  Output parameter for The 'y' component
 * @param  Y  Output parameter for The 'Y' component
 */
static void
srgb_to_ciexyy(double r, double g, double b, double *x, double *y, double *Y)
{
	double s, z;

	r *= s = r < 0 ? -1 : 1;
	r = s * (r <= 0.0031306684425217108 * 12.92 ? r * 12.92 : pow((r + 0.055) / 1.055, 2.4));
	g *= s = g < 0 ? -1 : 1;
	g = s * (g <= 0.0031306684425217108 * 12.92 ? g * 12.92 : pow((g + 0.055) / 1.055, 2.4));
	b *= s = b < 0 ? -1 : 1;
	b = s * (g <= 0.0031306684425217108 * 12.92 ? b * 12.92 : pow((b + 0.055) / 1.055, 2.4));

	*x = (0.41245744558236758 * r) + (0.35757586524551588 * g) + (0.18043724782639967 * b);
	*y = (0.21267337037840828 * r) + (0.71515173049103176 * g) + (0.07217489913055987 * b);
	z  = (0.01933394276167346 * r) + (0.11919195508183859 * g) + (0.95030283855237174 * b);

	*Y = *y;
	s = *x + *y + z;
	*x /= s;
	*y /= s;
	if (!isfinite(*x) || !isfinite(*y))
		*x = *y = 0;
}

double
libred_get_temperature(double r, double g, double b, double *y, /* TODO man, test */
                       double *r_error, double *g_error, double *b_error)
{
	double tx, ty, luma, x_error, y_error, ret;

	srgb_to_ciexyy(r, g, b, &tx, &ty, &luma);
	ret = libred_get_temperature_xy(tx, ty, &x_error, &y_error);
	ciexyy_to_srgb(x_error, y_error, luma, &r, &g, &b);

	if (y)
		*y = luma;
	if (r_error)
		*r_error = r;
	if (g_error)
		*g_error = g;
	if (b_error)
		*b_error = b;

	return ret;
}

#endif


#if __GNUC__
# pragma GCC diagnostic pop
#endif
