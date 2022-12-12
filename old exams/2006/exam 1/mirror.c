#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROTOCOL "http://"

static void usage(void) {
  fprintf(stderr,
          "Usage: mirror [-l] [-t timeout] [-m maximum] [-f filetype] url \n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) { exit(EXIT_SUCCESS); }