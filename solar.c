/* See LICENSE file for copyright and license details. */
#include "libred.h"
#include <math.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunsuffixed-float-constants"
#endif

/* Select clock. */
#if defined(DO_NOT_USE_COARSEC_CLOCK) || !defined(CLOCK_REALTIME_COARSE)
# ifdef CLOCK_REALTIME_COARSE
#  undef CLOCK_REALTIME_COARSE
# endif
# define CLOCK_REALTIME_COARSE  CLOCK_REALTIME
#endif


/**
 * Get current Julian Centuries time (100 Julian Days since J2000)
 * and Julian Day time
 * 
 * @param   tc_out  Output parameter for the current Julian Centuries time
 * @param   td_out  Output parameter for the current Julian Day time
 * @return          0 on success, -1 on failure
 * @throws          Any error specified for clock_gettime(3) on error
 */
static int
julian_time(double *tc_out, double *td_out)
{
	struct timespec now;
	double tu;
	if (clock_gettime(CLOCK_REALTIME_COARSE, &now))
		return -1;
	tu = fma((double)now.tv_nsec, 0.000000001, (double)now.tv_sec);
	*td_out = tu / 86400.0 + 2440587.5;
	*tc_out = (*td_out - 2451545.0) / 36525.0;
	return 0;
}


/**
 * Convert an angle (or otherwise) from degrees to radians
 * 
 * @param  deg  The angle in degrees
 * @param       The angle in radians
 */
static double
radians(double deg)
{
	return (double)M_PI / 180.0 * deg;
}

/**
 * Convert an angle (or otherwise) from radians to degrees
 * 
 * @param  rad  The angle in radians
 * @param       The angle in degrees
 */
static double
degrees(double rad)
{
	return 180.0 / (double)M_PI * rad;
}

/**
 * Convert an angle (or otherwise) from degrees to radians
 * and, using fused multply–add, add some number of degrees
 * 
 * @param  deg  The angle in degrees
 * @param  aug  The number of radians to add
 * @param       The angle in radians, plus `aug`
 */
static double
radians_plus(double deg, double aug)
{
	return fma((double)M_PI / 180.0, deg, aug);
}

/**
 * Convert an angle (or otherwise) from radians to degrees
 * and, using fused multply–add, add some number of degrees
 * 
 * @param  rad  The angle in radians
 * @param  aug  The number of degrees to add
 * @param       The angle in degrees, plus `aug`
 */
static double
degrees_plus(double rad, double aug)
{
	return fma(180.0 / (double)M_PI, rad, aug);
}


/**
 * Calculates the Sun's elevation from the solar hour angle
 * 
 * @param   latitude     The latitude in degrees northwards from 
 *                       the equator, negative for southwards
 * @param   declination  The declination, in radians
 * @param   hour_angle   The solar hour angle, in radians
 * @return               The Sun's elevation, in radians
 */
static double
elevation_from_hour_angle(double latitude, double declination, double hour_angle)
{
	double c, s;
	latitude = radians(latitude);
	c = cos(latitude) * cos(declination);
	s = sin(latitude) * sin(declination);
	return asin(fma(c, cos(hour_angle), s));
}

/**
 * Calculates the Sun's geometric mean longitude
 * 
 * @param   t  The time in Julian Centuries
 * @return     The Sun's geometric mean longitude in radians
 */
static double
sun_geometric_mean_longitude(double t)
{
	return radians(fmod(fma(fma(0.0003032, t, 36000.76983), t, 280.46646), 360.0));
}

/**
 * Calculates the Sun's geometric mean anomaly
 * 
 * @param   t  The time in Julian Centuries
 * @return     The Sun's geometric mean anomaly in radians
 */
static double
sun_geometric_mean_anomaly(double t)
{
	return radians(fmod(fma(fma(-0.0001537, t, 35999.05029), t, 357.52911), 360.0));
}

/**
 * Calculates the Earth's orbit eccentricity
 * 
 * @param   t  The time in Julian Centuries
 * @return     The Earth's orbit eccentricity
 */
static double
earth_orbit_eccentricity(double t)
{
	return fma(fma(-0.0000001267, t, -0.000042037), t, 0.016708634);
}

/**
 * Calculates the Sun's equation of the centre, the difference
 * between the true anomaly and the mean anomaly
 * 
 * @param   t  The time in Julian Centuries
 * @return     The Sun's equation of the centre, in radians
 */
static double
sun_equation_of_centre(double t)
{
	double a = sun_geometric_mean_anomaly(t), r;
	r = sin(1.0 * a) * fma(fma(-0.000014, t, -0.004817), t, 1.914602);
	r = fma(sin(2.0 * a), fma(-0.000101, t, 0.019993), r);
	r = fma(sin(3.0 * a), 0.000289, r);
	return radians(r);
}

/**
 * Calculates the Sun's real longitudinal position
 * 
 * @param   t  The time in Julian Centuries
 * @return     The longitude, in radians
 */
static double
sun_real_longitude(double t)
{
	return sun_geometric_mean_longitude(t) + sun_equation_of_centre(t);
}

/**
 * Calculates the Sun's apparent longitudinal position
 * 
 * @param   t  The time in Julian Centuries
 * @return     The longitude, in radians
 */
static double
sun_apparent_longitude(double t)
{
	double r = degrees_plus(sun_real_longitude(t), -0.00569);
	double a = radians(fma(-1934.136, t, 125.04));
	return radians(fma(-0.00478, sin(a), r));
}

/**
 * Calculates the mean ecliptic obliquity of the Sun's
 * apparent motion without variation correction
 * 
 * @param   t  The time in Julian Centuries
 * @return     The uncorrected mean obliquity, in radians
 */
static double
mean_ecliptic_obliquity(double t)
{
	double r = fma(fma(fma(0.001813, t, -0.00059), t, -46.815), t, 21.448);
	return radians(23.0 + (26.0 + r / 60.0) / 60.0);
}

/**
 * Calculates the mean ecliptic obliquity of the Sun's
 * parent motion with variation correction
 * 
 * @param   t  The time in Julian Centuries
 * @return     The mean obliquity, in radians
 */
static double
corrected_mean_ecliptic_obliquity(double t)
{
	double r = cos(radians(fma(-1934.136, t, 125.04)));
	return radians_plus(0.00256 * r, mean_ecliptic_obliquity(t));
}

/**
 * Calculates the Sun's declination
 * 
 * @param   t  The time in Julian Centuries
 * @return     The Sun's declination, in radian
 */
static double
solar_declination(double t)
{
	double r = sin(corrected_mean_ecliptic_obliquity(t));
	return asin(r * sin(sun_apparent_longitude(t)));
}

/**
 * Calculates the equation of time, the discrepancy
 * between apparent and mean solar time
 * 
 * @param   t  The time in Julian Centuries
 * @return     The equation of time, in minutes of time
 */
static double
equation_of_time(double t)
{
	double l = sun_geometric_mean_longitude(t);
	double e = earth_orbit_eccentricity(t);
	double m = sun_geometric_mean_anomaly(t);
	double y = tan(corrected_mean_ecliptic_obliquity(t) / 2.0);
	double r, c, s;
	y *= y;
	s = y * sin(2.0 * l);
	c = y * cos(2.0 * l);
	r = fma(fma(4.0, c, -2.0), e * sin(m), s);
	r = fma(-0.5 * y*y, sin(4.0 * l), r);
	r = fma(-1.25 * e*e, sin(2.0 * m), r);
	return 4.0 * degrees(r);
}

/**
 * Calculates the Sun's elevation as apparent
 * from a geographical position
 * 
 * @param   tc         The time in Julian Centuries
 * @param   td         The time in Julian Days
 * @param   latitude   The latitude in degrees northwards from 
 *                     the equator, negative for southwards
 * @param   longitude  The longitude in degrees eastwards from
 *                     Greenwich, negative for westwards
 * @return             The Sun's apparent elevation at the specified time as seen
 *                     from the specified position, measured in radians
 */
static double
solar_elevation_from_time(double tc, double td, double latitude, double longitude)
{
	double r;
	td = td - round(td);
	r = fma(1440, td - 1, -equation_of_time(tc));
	r = radians(fma(0.25, r, -longitude));
	return elevation_from_hour_angle(latitude, solar_declination(tc), r);
}

/**
 * Calculates the Sun's elevation as apparent
 * from a geographical position
 * 
 * @param   latitude   The latitude in degrees northwards from 
 *                     the equator, negative for southwards
 * @param   longitude  The longitude in degrees eastwards from
 *                     Greenwich, negative for westwards
 * @param   elevation  Output parameter for the Sun's apparent elevation
 *                     as seen, right now, from the specified position,
 *                     measured in degrees
 * @return             0 on success, -1 on failure
 * @throws             Any error specified for clock_gettime(3) on error
 */
double
libred_solar_elevation(double latitude, double longitude, double *elevation)
{
	double tc, td;
	if (julian_time(&tc, &td))
		return -1;
	*elevation = degrees(solar_elevation_from_time(tc, td, latitude, longitude));
	return 0;
}

/**
 * This function is obsolete
 */
int
libred_check_timetravel(void)
{
	return 0;
}
