#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "common.h"

static volatile sig_atomic_t quit = 0;

static void onSignal(int signal) {
  log("Received signal %d\n", signal);
  quit = 1;
}

int main(int argc, char **argv) {
  if (argc > 1) {
    argumentError("No arguments allowed");
  }

  // register signal handler
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = onSignal;
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);

  // create shared memory -> shm_open()
  // set shared memory size -> ftruncate()
  // map shared memory into memory -> mmap()

  // create semaphores -> sem_open()

  while (!quit) {
    // do stuff (mutual exclusion)
    break;
  }

  // unmap shared memory -> munmap()
  // close shared memory -> close()
  // unlink shared memory -> shm_unlink()

  // close semaphores -> sem_close()
  // unlink semaphores -> sem_unlink()

  return EXIT_SUCCESS;
}