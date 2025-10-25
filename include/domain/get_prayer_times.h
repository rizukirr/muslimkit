/**
 * @file get_prayer_times.h
 * @brief Prayer times data structures and fetching functionality
 *
 * This header defines the data models for Islamic prayer times retrieved from
 * the MyQuran API. It includes structures for prayer timings, date information
 * (both Hijri and Gregorian calendars), location metadata, and calculation methods.
 */

#ifndef GET_PRAYER_TIMES_H
#define GET_PRAYER_TIMES_H

#include "../network/connection.h"
#include <stdbool.h>

/**
 * @brief Prayer time timings for a single day
 *
 * Contains the times for all five daily Islamic prayers plus sunrise.
 * All times are represented as strings in HH:MM format (24-hour time).
 */
struct timings_s {
  char *Fajr;     /**< Time for Fajr (dawn) prayer */
  char *Sunrise;  /**< Sunrise time (not a prayer, but marks end of Fajr period) */
  char *Dhuhr;    /**< Time for Dhuhr (noon) prayer */
  char *Asr;      /**< Time for Asr (afternoon) prayer */
  char *Maghrib;  /**< Time for Maghrib (sunset) prayer */
  char *Isha;     /**< Time for Isha (night) prayer */
};

/**
 * @brief Month representation
 *
 * Represents a calendar month used in both Hijri and Gregorian date structures.
 */
struct month_s {
  int number;  /**< Month number (1-12) */
};

/**
 * @brief Islamic (Hijri) calendar date information
 *
 * Contains the date in the Islamic calendar system, which is based on lunar months.
 * The Hijri calendar is used to determine Islamic holidays and prayer times.
 */
struct hijri_s {
  char *date;             /**< Full date string in Hijri calendar (e.g., "15-03-1446") */
  char *format;           /**< Date format specification string */
  char *day;              /**< Day name in Hijri calendar (e.g., "Al Juma'a") */
  struct month_s month;   /**< Month information in Hijri calendar */
  char *year;             /**< Hijri year as string (e.g., "1446") */
};

/**
 * @brief Gregorian calendar date information
 *
 * Contains the date in the standard Gregorian calendar system.
 */
struct georgian_s {
  char *date;             /**< Full date string in Gregorian calendar (e.g., "20-09-2024") */
  char *format;           /**< Date format specification string */
  char *day;              /**< Day name in Gregorian calendar (e.g., "Friday") */
  struct month_s month;   /**< Month information in Gregorian calendar */
  char year;              /**< Gregorian year (Note: likely should be char* or int) */
};

/**
 * @brief Complete date information
 *
 * Combines both Hijri and Gregorian calendar representations of a single date,
 * along with human-readable and timestamp formats.
 */
struct date_s {
  char *readable;           /**< Human-readable date string */
  char *timestamp;          /**< Unix timestamp or ISO 8601 timestamp string */
  struct hijri_s hijri;     /**< Date in Islamic (Hijri) calendar */
  struct georgian_s georgian;  /**< Date in Gregorian calendar */
};

/**
 * @brief Prayer time calculation method
 *
 * Describes the mathematical method used to calculate prayer times.
 * Different Islamic organizations use different calculation methods
 * based on sun angles and geographical considerations.
 */
struct method_s {
  int id;       /**< Numerical identifier for the calculation method */
  char *name;   /**< Name of the calculation method (e.g., "JAKIM", "MWL") */
};

/**
 * @brief Metadata for prayer time calculation
 *
 * Contains geographical coordinates, timezone information, and the
 * calculation method used to determine the prayer times.
 */
struct meta_s {
  double latitude;        /**< Latitude of the location in decimal degrees */
  double longitude;       /**< Longitude of the location in decimal degrees */
  char *timezone;         /**< Timezone identifier (e.g., "Asia/Jakarta") */
  struct method_s method; /**< Calculation method used for prayer times */
};

/**
 * @brief Complete prayer time data for a single day
 *
 * Aggregates all information about prayer times for a specific location and date,
 * including the actual prayer timings, date information, and calculation metadata.
 */
struct data_s {
  struct timings_s timings;  /**< Prayer time schedules for the day */
  struct date_s date;        /**< Date information in multiple calendar systems */
  struct meta_s meta;        /**< Location and calculation metadata */
};

/**
 * @brief API response wrapper for prayer times
 *
 * Top-level structure representing the complete API response from the
 * MyQuran prayer times endpoint. Contains status information and the
 * actual prayer time data.
 */
struct prayer_times_s {
  int code;           /**< HTTP response code from the API */
  bool status;        /**< Success status (true if request succeeded) */
  struct data_s data; /**< The actual prayer time data */
};

/**
 * @brief Fetch prayer times for a specific city
 *
 * Retrieves Islamic prayer times for a given Indonesian city from the MyQuran API.
 * The function makes an HTTPS request to api.myquran.com and stores the raw JSON
 * response in the provided buffer.
 *
 * @param city_id City identifier obtained from get_cities() (e.g., "1301")
 * @param response Buffer to store the raw JSON response from the API
 * @param response_size Size of the response buffer in bytes
 * @return 0 on success, non-zero error code on failure
 *
 * @note The response buffer must be allocated by the caller and should be
 *       large enough to hold the complete JSON response (typically 2-4 KB).
 * @warning This function is currently a stub and not yet implemented.
 *
 * Example usage:
 * @code
 *   char buffer[8192];
 *   int result = get_prayer_times("1301", buffer, sizeof(buffer));
 *   if (result == 0) {
 *     // Parse buffer with prayer time JSON parser
 *   }
 * @endcode
 */
int get_prayer_times(char *city_id, char *response, int response_size);

#endif
