#ifndef TMUTILS_H
#define TMUTILS_H

struct tmutils {
  int year;
  int month;
  int days;
  int hours;
  int minutes;
  int seconds;
};

void get_current_time(struct tmutils *dest);

#endif
