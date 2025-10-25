#include "utils/tmutils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void get_current_time(struct tmutils *dest) {
  if (dest == NULL) {
    fprintf(stderr, "get_current_time ERROR: destination NULL");
    return;
  }
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  memset(dest, 0, sizeof(*dest));

  dest->year = t->tm_year + 1900;
  dest->month = t->tm_mon + 1;
  dest->days = t->tm_mday;
  dest->hours = t->tm_hour;
  dest->minutes = t->tm_min;
  dest->seconds = t->tm_sec;
}
