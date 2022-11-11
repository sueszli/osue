#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

int main(int argc, char **argv) {
  if (argc > 1) {
    argumentError("No arguments allowed");
  }

  return EXIT_SUCCESS;
}