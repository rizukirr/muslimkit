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

#include <stdbool.h>

struct prayer_times_req {
  char *path;
};

struct prayer_times_data_schedule {
  char *date;
  char *fajr;
  char *dhuha;
  char *dzuhr;
  char *ashr;
  char *maghrib;
  char *isya;
};

struct prayer_times_data {
  int id;
  char *location;
  char *province;
  int schedule_size;
  struct prayer_times_data_schedule *schedule;
};

struct prayer_times {
  bool status;
  struct prayer_times_req req;
  struct prayer_times_data data;
};

int get_prayer_times(const char *city_id, struct prayer_times *dest);

void get_prayer_times_free(struct prayer_times *prayer_t);

#endif
