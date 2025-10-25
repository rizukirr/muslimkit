#include "utils/strutils.h"
#include <ctype.h>
#include <stdio.h>

void to_lowercase(char *str) {
  if (str == NULL) {
    printf("str NULL");
    return;
  }

  for (int i = 0; str[i]; i++) {
    str[i] = tolower(str[i]);
  }
}
