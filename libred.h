/* See LICENSE file for copyright and license details. */
#ifndef LIBRED_H
#define LIBRED_H



/**
 * Approximate apparent size of the Sun in degrees
 */
#define LIBRED_SOLAR_APPARENT_RADIUS  (32.0 / 60.0)


/**
 * The Sun's elevation at sunset and sunrise, measured in degrees
 */
#define LIBRED_SOLAR_ELEVATION_SUNSET_SUNRISE   (-32.0 / 60.0)

/**
 * The Sun's elevation at civil dusk and civil dawn, measured in degrees
 */
#define LIBRED_SOLAR_ELEVATION_CIVIL_DUSK_DAWN   (-6.0)

/**
 * The Sun's elevation at nautical dusk and nautical dawn, measured in degrees
 */
#define LIBRED_SOLAR_ELEVATION_NAUTICAL_DUSK_DAWN  (-12.0)

/**
 * The Sun's elevation at astronomical dusk and astronomical dawn, measured in degrees
 */
#define LIBRED_SOLAR_ELEVATION_ASTRONOMICAL_DUSK_DAWN  (-18.0)

/**
 * The Sun's elevation at amateur astronomical dusk and amateur astronomical dawn, measured in degrees
 */
#define LIBRED_SOLAR_ELEVATION_AMATEUR_ASTRONOMICAL_DUSK_DAWN  (-15.0)

/**
 * The Sun's lowest elevation during the golden hour, measured in degrees
 */
#define LIBRED_SOLAR_ELEVATION_GOLDEN_HOUR_LOW  (-4.0)

/**
 * The Sun's highest elevation during the golden hour, measured in degrees
 */
#define LIBRED_SOLAR_ELEVATION_GOLDEN_HOUR_HIGH  (6.0)

/**
 * The Sun's lowest elevation during the blue hour, measured in degrees
 */
#define LIBRED_SOLAR_ELEVATION_BLUE_HOUR_LOW  (-6.0)

/**
 * The Sun's highest elevation during the blue hour, measured in degrees
 */
#define LIBRED_SOLAR_ELEVATION_BLUE_HOUR_HIGH  (-4.0)


/**
 * Test whether it is twilight
 * 
 * @param   ELEV:double  The current elevation
 * @return               1 if is twilight, 0 otherwise
 */
#define LIBRED_IS_TWILIGHT(ELEV)  ((-18.0 <= (ELEV)) && ((ELEV) <= -32.0 / 60.0))

/**
 * Test whether it is civil twilight
 * 
 * @param   ELEV:double  The current elevation
 * @return               1 if is civil twilight, 0 otherwise
 */
#define LIBRED_IS_CIVIL_TWILIGHT(ELEV)  ((-6.0 <= (ELEV)) && ((ELEV) <= -32.0 / 60.0))

/**
 * Test whether it is nautical twilight
 * 
 * @param   ELEV:double  The current elevation
 * @return               1 if is nautical twilight, 0 otherwise
 */
#define LIBRED_IS_NAUTICAL_TWILIGHT(ELEV)  ((-12.0 <= (ELEV)) && ((ELEV) <= -32.0 / 60.0))

/**
 * Test whether it is astronomical twilight
 * 
 * @param   ELEV:double  The current elevation
 * @return               1 if is astronomical twilight, 0 otherwise
 */
#define LIBRED_IS_ASTRONOMICAL_TWILIGHT(ELEV)  ((-18.0 <= (ELEV)) && ((ELEV) <= -32.0 / 60.0))

/**
 * Test whether it is amateur astronomical twilight
 * 
 * @param   ELEV:double  The current elevation
 * @return               1 if is amatuer astronomical twilight, 0 otherwise
 */
#define LIBRED_IS_AMATEUR_ASTRONOMICAL_TWILIGHT(ELEV)  ((-18.0 <= (ELEV)) && ((ELEV) <= -15.0))

/**
 * Test whether it is nighttime
 * 
 * @param   ELEV:double  The current elevation
 * @return               1 if is nighttime, 0 otherwise
 */
#define LIBRED_IS_NIGHTTIME(ELEV)  ((ELEV) < -18.0)

/**
 * Test whether it is daytime
 * 
 * @param   ELEV:double  The current elevation
 * @return               1 if is daytime, 0 otherwise
 */
#define LIBRED_IS_DAYTIME(ELEV)  ((ELEV) > -32.0 / 60.0)

/**
 * Test whether it is the golden hour
 * 
 * @param   ELEV:double  The current elevation
 * @return               1 if is golden hour, 0 otherwise
 */
#define LIBRED_IS_GOLDEN_HOUR(ELEV)  ((-4.0 <= (ELEV)) && ((ELEV) <= 6.0))

/**
 * Test whether it is the blue hour
 * 
 * @param   ELEV:double  The current elevation
 * @return               1 if is blue hour, 0 otherwise
 */
#define LIBRED_IS_BLUE_HOUR(ELEV)  ((-6.0 <= (ELEV)) && ((ELEV) <= -4.0))


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
double libred_solar_elevation(double, double, double *);


/**
 * Exit if time the is before year 0 in J2000
 * 
 * @return  0 on success, -1 on error
 */
int libred_check_timetravel(void);



/**
 * The highest colour temperature in the table
 */
#define LIBRED_HIGHEST_TEMPERATURE  40000

/**
 * The lowest colour temperature in the table
 */
#define LIBRED_LOWEST_TEMPERATURE  1000

/**
 * The temperature difference between the colours in the table.
 * Note, `libred_get_colour` will make interpolation for colours
 * that are not in the table
 */
#define LIBRED_DELTA_TEMPERATURE  100


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
 * 
 * @throws  EDOM  The selected temperature is below 1000K
 */
int libred_get_colour(long int, double *, double *, double *);



#endif
