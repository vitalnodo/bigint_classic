#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ltrim(char* s) {
  if (s && *s) {
    char *res = s;
    while ('0' == *res) {
        ++res;
    }

    if (res == s) {
      return;
    }

    if ('\0' == *res) {
        --res;
    }

    memmove(s, res, strlen(res) + 1);
  }
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
    printf("Actual  : %s\n", actual);

    printf("Compare : ");
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