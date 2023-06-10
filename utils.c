#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *ltrim(char *const s) {
  if (s && *s) {
    size_t len = strlen(s);
    char *cur = s;

    while (*cur && *cur == '0') {
      ++cur;
      --len;
    }

    if (s != cur)
      memmove(s, cur, len + 1);
  }

  return s;
}

char char2digit(char ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  } else if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  } else if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  } else {
    return -1;
  }
}

void check(char *expected, char *actual) {
  int result = strcmp(expected, actual);

  if (result == 0) {
    return;
  } else {
    int i, len = strlen(expected);

    printf("\nExpected: %s\n", expected);

    printf("Actual  : ");
    for (i = 0; i < len; i++) {
      if (expected[i] == actual[i]) {
        printf("\033[32m%c\033[0m", expected[i]);
      } else {
        printf("\033[31m%c\033[0m", actual[i]);
      }
    }
    while (actual[i] != '\0') {
      printf("\033[31m%c\033[0m", actual[i]);
      i++;
    }
    printf("\n");
    exit(EXIT_FAILURE);
  }
}