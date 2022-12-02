#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#define errorUsage(msg)                                                 \
  do {                                                                  \
    fprintf(stderr, "Wrong usage: %s\nSYNOPSIS:\n\tsupervisor\n", msg); \
    exit(EXIT_FAILURE);                                                 \
  } while (0);

static volatile sig_atomic_t quit = 0;
static void onSignal(int signum) { quit = 1; }

static void initSignalListener(void) {
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = onSignal;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    errorHandler("sigaction");
  }
  if (sigaction(SIGQUIT, &sa, NULL) == -1) {
    errorHandler("sigaction");
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    errorHandler("sigaction");
  }
}

int main(int argc, char *argv[]) {
  printf("\n\n");

  if (argc > 1) {
    errorUsage("no arguments allowed");
  }

  initSignalListener();

  // shm_open() -> read man pages!

  // ftruncate()

  // mmap()

  // sem_open()

  while (!quit) {
  }

  // sem_unlink()

  // sem_close()

  // munmap()

  // shm_unlink()

  // close(shm)

  printf("\n\n");
  return EXIT_SUCCESS;
}