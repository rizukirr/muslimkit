/**
 * @file get_cities.h
 * @brief API for fetching and parsing city data for prayer times.
 *
 * Provides functions to retrieve the list of available cities from the
 * prayer times API, parse the JSON response, and manage associated data
 * structures.
 */

#ifndef GET_CITIES_H
#define GET_CITIES_H

#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHUNK_SIZE 4096

/**
 * @brief Structure representing a single city data entry.
 *
 * Contains city information returned by the prayer times API.
 */
struct cities_data_s {
  char *id;     /**< City ID */
  char *lokasi; /**< City location/name */
};

/**
 * @brief Structure representing the complete cities response.
 *
 * Holds the parsed JSON response containing an array of cities
 * and the request status.
 */
struct cities_s {
  bool status;                /**< API request status */
  struct cities_data_s *data; /**< Array of city data */
  size_t size;                /**< Number of cities in the data array */
};

/**
 * @brief Free memory allocated for cities structure.
 *
 * Releases all memory associated with a cities_s structure, including
 * the data array and individual city entries.
 *
 * @param cities  Pointer to the cities_s structure to free.
 */
void get_city_free(struct cities_s *cities);

/**
 * @brief Fetch cities data from the API.
 *
 * Makes an HTTPS request to the prayer times API to retrieve the list
 * of all available cities. Handles SSL connection, HTTP request/response.
 *
 * @return Returns a pointer to the raw HTTP response as a string,
 *         or NULL on failure (connection error, SSL error, etc.).
 *
 * @warning The returned pointer must be freed using get_cities_free().
 */
int get_city(struct cities_s *dest);

#endif
