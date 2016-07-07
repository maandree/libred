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
#ifndef LIBRED_H
#define LIBRED_H



/**
 * Approximate apparent size of the Sun in degrees.
 */
#define LIBRED_SOLAR_APPARENT_RADIUS  (32.0 / 60.0)


/**
 * The Sun's elevation at sunset and sunrise, measured in degrees.
 */
#define LIBRED_SOLAR_ELEVATION_SUNSET_SUNRISE   (-32.0 / 60.0)

/**
 * The Sun's elevation at civil dusk and civil dawn, measured in degrees.
 */
#define LIBRED_SOLAR_ELEVATION_CIVIL_DUSK_DAWN   (-6.0)

/**
 * The Sun's elevation at nautical dusk and nautical dawn, measured in degrees.
 */
#define LIBRED_SOLAR_ELEVATION_NAUTICAL_DUSK_DAWN  (-12.0)

/**
 * The Sun's elevation at astronomical dusk and astronomical dawn, measured in degrees.
 */
#define LIBRED_SOLAR_ELEVATION_ASTRONOMICAL_DUSK_DAWN  (-18.0)

/**
 * The Sun's elevation at amateur astronomical dusk and amateur astronomical dawn, measured in degrees.
 */
#define LIBRED_SOLAR_ELEVATION_AMATEUR_ASTRONOMICAL_DUSK_DAWN  (-15.0)

/**
 * The Sun's lowest elevation during the golden hour, measured in degrees.
 */
#define LIBRED_SOLAR_ELEVATION_GOLDEN_HOUR_LOW  (-4.0)

/**
 * The Sun's highest elevation during the golden hour, measured in degrees.
 */
#define LIBRED_SOLAR_ELEVATION_GOLDEN_HOUR_HIGH  (6.0)

/**
 * The Sun's lowest elevation during the blue hour, measured in degrees.
 */
#define LIBRED_SOLAR_ELEVATION_BLUE_HOUR_LOW  (-6.0)

/**
 * The Sun's highest elevation during the blue hour, measured in degrees.
 */
#define LIBRED_SOLAR_ELEVATION_BLUE_HOUR_HIGH  (-4.0)


/**
 * Test whether it is twilight.
 * 
 * @param   ELEV:double  The current elevation.
 * @return               1 if is twilight, 0 otherwise.
 */
#define LIBRED_IS_TWILIGHT(ELEV)  ((-18.0 <= (ELEV)) && ((ELEV) <= -32.0 / 60.0))

/**
 * Test whether it is civil twilight.
 * 
 * @param   ELEV:double  The current elevation.
 * @return               1 if is civil twilight, 0 otherwise.
 */
#define LIBRED_IS_CIVIL_TWILIGHT(ELEV)  ((-6.0 <= (ELEV)) && ((ELEV) <= -32.0 / 60.0))

/**
 * Test whether it is nautical twilight.
 * 
 * @param   ELEV:double  The current elevation.
 * @return               1 if is nautical twilight, 0 otherwise.
 */
#define LIBRED_IS_NAUTICAL_TWILIGHT(ELEV)  ((-12.0 <= (ELEV)) && ((ELEV) <= -32.0 / 60.0))

/**
 * Test whether it is astronomical twilight.
 * 
 * @param   ELEV:double  The current elevation.
 * @return               1 if is astronomical twilight, 0 otherwise.
 */
#define LIBRED_IS_ASTRONOMICAL_TWILIGHT(ELEV)  ((-18.0 <= (ELEV)) && ((ELEV) <= -32.0 / 60.0))

/**
 * Test whether it is amateur astronomical twilight.
 * 
 * @param   ELEV:double  The current elevation.
 * @return               1 if is amatuer astronomical twilight, 0 otherwise.
 */
#define LIBRED_IS_AMATEUR_ASTRONOMICAL_TWILIGHT(ELEV)  ((-18.0 <= (ELEV)) && ((ELEV) <= -15.0))

/**
 * Test whether it is nighttime.
 * 
 * @param   ELEV:double  The current elevation.
 * @return               1 if is nighttime, 0 otherwise.
 */
#define LIBRED_IS_NIGHTTIME(ELEV)  ((ELEV) < -18.0)

/**
 * Test whether it is daytime.
 * 
 * @param   ELEV:double  The current elevation.
 * @return               1 if is daytime, 0 otherwise.
 */
#define LIBRED_IS_DAYTIME(ELEV)  ((ELEV) > -32.0 / 60.0)

/**
 * Test whether it is the golden hour.
 * 
 * @param   ELEV:double  The current elevation.
 * @return               1 if is golden hour, 0 otherwise.
 */
#define LIBRED_IS_GOLDEN_HOUR(ELEV)  ((-4.0 <= (ELEV)) && ((ELEV) <= 6.0))

/**
 * Test whether it is the blue hour.
 * 
 * @param   ELEV:double  The current elevation.
 * @return               1 if is blue hour, 0 otherwise.
 */
#define LIBRED_IS_BLUE_HOUR(ELEV)  ((-6.0 <= (ELEV)) && ((ELEV) <= -4.0))


/**
 * Calculates the Sun's elevation as apparent
 * from a geographical position.
 * 
 * @param   latitude   The latitude in degrees northwards from 
 *                     the equator, negative for southwards.
 * @param   longitude  The longitude in degrees eastwards from
 *                     Greenwich, negative for westwards.
 * @return             The Sun's apparent elevation as seen, right now,
 *                     from the specified position, measured in degrees.
 * 
 * @throws  0  On success.
 * @throws     Any error specified for clock_gettime(3) on error.
 */
double libred_solar_elevation(double, double);


/**
 * Exit if time the is before year 0 in J2000.
 * 
 * @return  0 on success, -1 on error.
 */
int libred_check_timetravel(void);



/**
 * The highest colour temperature in the table.
 */
#define LIBRED_HIGHEST_TEMPERATURE  40000

/**
 * The lowest colour temperature in the table.
 */
#define LIBRED_LOWEST_TEMPERATURE  1000

/**
 * The temperature difference between the colours in the table.
 * Note, `libred_get_colour` will make interpolation for colours
 * that are not in the table.
 */
#define LIBRED_DELTA_TEMPERATURE  100


/**
 * The file descriptor to the colour lookup table.
 * -1 if none is open.
 */
extern int libred_fd;

/**
 * Iff this macro is define `libred_fd` is available.
 */
#define LIBRED_HAVE_FD  1


/**
 * This function must be called, once,
 * before calling `libred_get_colour`.
 * 
 * @return  0 on success, -1 on error.
 * 
 * @throws  Any error specified for `open(3)`.
 */
int libred_init_colour(void);

/**
 * Call this when the process will not
 * longer make calls to `libred_get_colour`.
 */
void libred_term_colour(void);

/**
 * Get the [0, 1] sRGB values of a colour temperature.
 * 
 * @param   temp  The desired colour temperature.
 * @param   r     Output parameter for the “red” value.
 * @param   g     Output parameter for the green value.
 * @param   b     Output parameter for the blue value.
 * @return        0 on succeess, -1 on error.
 * 
 * @throws  EOVERFLOW  The file did not have the expected size.
 * @throws  EDOM       The selected temperature is below 1000 K.
 * @throws             Any error specified for pread(3).
 */
int libred_get_colour(long int, double*, double*, double*);



#endif

