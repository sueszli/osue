#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// print macros ::
#ifdef DEBUG
#define log(fmt, ...) \
  fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define log(fmt, ...) /* NOP */
#endif

#define error(s)                                   \
  fprintf(stderr, "%s: %s\n", s, strerror(errno)); \
  exit(EXIT_FAILURE);

#define argumentError(s)                   \
  fprintf(stderr, "%s\n", s);              \
  fprintf(stderr, "%s\n", "USAGE:");       \
  fprintf(stderr, "\t%s\n", "supervisor"); \
  exit(EXIT_FAILURE);
// :: print macros

int main(int argc, char **argv) {
  if (argc > 1) {
    argumentError("No arguments allowed");
  }
}