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
#define LINEAR_TO_SRGB(C)\
	do {\
		double sign = (C) < 0 ? -1 : 1;\
		(C) *= sign;\
		(C) = (((C) <= 0.0031306684425217108) ? (12.92 * (C)) : (1.055 * pow((C), 1.0 / 2.4) - 0.055));\
		(C) *= sign;\
	} while (0)

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
  
	/* Convert CIE XYZ to [0, 1] linear RGB */
	*r = ( 3.24044625464773750067593027779366821050643920898438 * X)
	   + (-1.53713476182008057513428411766653880476951599121094 * Y)
	   + (-0.49853019302272871815517873983480967581272125244141 * Z);
	*g = (-0.96926660624467975146956177923129871487617492675781 * X)
	   + ( 1.87601195978837020916785149893257766962051391601562 * Y)
	   + ( 0.04155604221443006535130493261931405868381261825562 * Z);
	*b = ( 0.05564350356435283223577314970498264301568269729614 * X)
	   + (-0.20402617973596023914772956686647376045584678649902 * Y)
	   + ( 1.05722656772270329206264705135254189372062683105469 * Z);

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

	/* Convert [0, 1] linear RGB to [0, 1] sRGB */
	LINEAR_TO_SRGB(*r);
	LINEAR_TO_SRGB(*g);
	LINEAR_TO_SRGB(*b);
}
#ifndef LIBRED_COMPILING_PARSER


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
#define SRGB_TO_LINEAR(C)\
	do {\
		double sign = (C) < 0 ? -1 : 1;\
		(C) *= sign;\
		(C) = (((C) <= 0.0031306684425217108 * 12.92) ? ((C) / 12.92) : pow(((C) + 0.055) / 1.055, 2.4));\
		(C) *= sign;\
	} while (0)

	/* Convert [0, 1] sRGB to [0, 1] linear RGB */
	double s, z;

	SRGB_TO_LINEAR(r);
	SRGB_TO_LINEAR(g);
	SRGB_TO_LINEAR(b);

	/* Convert [0, 1] linear RGB to CIE XYZ */
	*x = 0.41245744558236757670854899515688885003328323364258 * r
	   + 0.35757586524551587814357844763435423374176025390625 * g
	   + 0.18043724782639966597308500695362454280257225036621 * b;
	*y = 0.21267337037840827740353688568575307726860046386719 * r
	   + 0.71515173049103175628715689526870846748352050781250 * g
	   + 0.07217489913055986916479156434434116818010807037354 * b;
	z  = 0.01933394276167346020889326041469757910817861557007 * r
	   + 0.11919195508183859366635459764438564889132976531982 * g
	   + 0.95030283855237174250873977143783122301101684570312 * b;
	*Y = z;

	/* Convert CIE XYZ to CIE xyY */
	*Y = *y;
	s = *x + *y + z;
	*x /= s;
	*y /= s;
	if (!isfinite(*x) || !isfinite(*y))
		*x = *y = 0;
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
interpolate(double x1, double y1, double x2, double y2, double temp, double *x, double *y)
{
	double weight = fmod(temp - (LIBRED_LOWEST_TEMPERATURE % LIBRED_DELTA_TEMPERATURE),
	                     (double)LIBRED_DELTA_TEMPERATURE) / (double)LIBRED_DELTA_TEMPERATURE;
	*x = x1 * (1 - weight) + x2 * weight;
	*y = y1 * (1 - weight) + y2 * weight;
}


int
libred_get_colour_xy(long int temp, double *x, double *y)
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
		interpolate(x1, y1, x2, y2, (double)temp, x, y);
	} else {
		*x = xy_table[i].x;
		*y = xy_table[i].y;
	}

	return 0;
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
		double x, y;
		x1 = xy_table[i].x;
		y1 = xy_table[i].y;
		x2 = xy_table[i + 1].x;
		y2 = xy_table[i + 1].y;
		interpolate(x1, y1, x2, y2, (double)temp, &x, &y);
		ciexyy_to_srgb(x, y, 1, r, g, b);
	} else {
		*r = rgb_table[i].r;
		*g = rgb_table[i].g;
		*b = rgb_table[i].b;
	}

	return 0;
}


double
libred_get_temperature_xy(double x, double y, double *x_error, double *y_error)
{
	size_t i = 0, j;
	double x1, y1, x2, y2, dx, dy, xd, yd, t, d2;
	double best_temp = -1, best_d2 = INFINITY;

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

		i = j;
	}

	return best_temp * LIBRED_DELTA_TEMPERATURE + LIBRED_LOWEST_TEMPERATURE;
}


double
libred_get_temperature(double r, double g, double b, double *y,
                       double *r_error, double *g_error, double *b_error)
{
	double tx, ty, luma, x_error, y_error, ret;

	srgb_to_ciexyy(r, g, b, &tx, &ty, &luma);
	ret = libred_get_temperature_xy(tx, ty, &x_error, &y_error);
	ciexyy_to_srgb(x_error, y_error, 1, &r, &g, &b);

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
