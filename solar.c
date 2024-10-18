/* See LICENSE file for copyright and license details. */
#include "libred.h"
#include <math.h>
#include <time.h>
#include <errno.h>
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
 * Get current Julian Centuries time (100 Julian days since J2000)
 * 
 * @param   nowp  Output parameter for the current Julian Centuries time
 * @return        0 on success, -1 on failure
 * @throws        Any error specified for clock_gettime(3) on error
 */
static int
julian_centuries(double *nowp)
{
	struct timespec now;
	if (clock_gettime(CLOCK_REALTIME_COARSE, &now))
		return -1;
	*nowp = (double)(now.tv_nsec) / 1000000000.0 + (double)(now.tv_sec);
	*nowp = (*nowp / 86400.0 + 2440587.5 - 2451545.0) / 36525.0;
	return 0;
}

/**
 * Convert a Julian Centuries timestamp to a Julian Day timestamp
 * 
 * @param   t  The time in Julian Centuries
 * @return      The time in Julian Days
 */
static double
julian_centuries_to_julian_day(double t)
{
	return 36525.0 * t + 2451545.0;
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
	return deg * (double)M_PI / 180.0;
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
	return rad * 180.0 / (double)M_PI;
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
	double r = cos(radians(latitude));
	r *= cos(hour_angle) * cos(declination);
	r += sin(radians(latitude)) * sin(declination);
	return asin(r);
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
	double r = fmod(pow(0.0003032 * t, 2.0) + 36000.76983 * t + 280.46646, 360.0);
#if defined(TIMETRAVELLER)
	r = r < 0.0 ? (r + 360.0) : r;
#endif
	return radians(r);
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
	return radians(pow(-0.0001537 * t, 2.0) + 35999.05029 * t + 357.52911);
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
	return pow(-0.0000001267 * t, 2.0) - 0.000042037 * t + 0.016708634;
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
	r  = sin(1.0 * a) * (pow(-0.000014 * t, 2.0) - 0.004817 * t + 1.914602);
	r += sin(2.0 * a) * (-0.000101 * t + 0.019993);
	r += sin(3.0 * a) * 0.000289;
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
	double r = degrees(sun_real_longitude(t)) - 0.00569;
	return radians(r - 0.00478 * sin(radians(-1934.136 * t + 125.04)));
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
	double r = pow(0.001813 * t, 3.0) - pow(0.00059 * t, 2.0) - 46.815 * t + 21.448;
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
	double r = 0.00256 * cos(radians(-1934.136 * t + 125.04));
	return radians(r + degrees(mean_ecliptic_obliquity(t)));
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
 * @return     The equation of time, in degrees
 */
static double
equation_of_time(double t)
{
	double l = sun_geometric_mean_longitude(t);
	double e = earth_orbit_eccentricity(t);
	double m = sun_geometric_mean_anomaly(t);
	double y = pow(tan(corrected_mean_ecliptic_obliquity(t) / 2.0), 2.0);
	double r = y * sin(2.0 * l);
	r += (4.0 * y * cos(2.0 * l) - 2.0) * e * sin(m);
	r -= pow(0.5 * y, 2.0) * sin(4.0 * l);
	r -= pow(1.25 * e, 2.0) * sin(2.0 * m);
	return 4.0 * degrees(r);
}

/**
 * Calculates the Sun's elevation as apparent
 * from a geographical position
 * 
 * @param   t          The time in Julian Centuries
 * @param   latitude   The latitude in degrees northwards from 
 *                     the equator, negative for southwards
 * @param   longitude  The longitude in degrees eastwards from
 *                     Greenwich, negative for westwards
 * @return             The Sun's apparent elevation at the specified time as seen
 *                     from the specified position, measured in radians
 */
static double
solar_elevation_from_time(double t, double latitude, double longitude)
{
	double a = julian_centuries_to_julian_day(t);
	a = (a - round(a) - 0.5) * 1440;
	a = 720.0 - a - equation_of_time(t);
	a = radians(a / 4.0 - longitude);
	return elevation_from_hour_angle(latitude, solar_declination(t), a);
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
	double t;
	if (julian_centuries(&t))
		return -1;
	*elevation = degrees(solar_elevation_from_time(t, latitude, longitude));
	return 0;
}

/**
 * Exit if time the is before year 0 in J2000
 * 
 * @return  0 on success, -1 on error
 */
int
libred_check_timetravel(void)
{
#if !defined(TIMETRAVELLER)
	struct timespec now;
	if (clock_gettime(CLOCK_REALTIME, &now))
		return -1;
	if (now.tv_sec < (time_t)946728000L) {
		fprintf(stderr,
		       "We have detected that you are a time-traveller"
		       "(or your clock is not configured correctly.)"
		       "Please recompile libred with -DTIMETRAVELLER"
		       "(or correct your clock.)");
		exit(1);
	}
#endif
	return 0;
}
