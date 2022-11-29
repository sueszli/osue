/**
 * @file supverisor.c
 * @author Yahya Jabary <mailto: yahya.jabary@tuwien.ac.at>
 * @date 09.11.2022
 *
 * @brief Manager process for generators (workers) which receives solutions and
 * updates the best one so far. Kills all workers when a signal from the user is
 * received.
 *
 * @details This manager process consists of 3 segments:
 * First, the Initialization: Creating the shared memory used for
 * communication between manager and workers and semaphores used to synchronize
 * all processes.
 * Second, the Main Loop: Receiving all submitted solutions from the workers and
 * logging them if they are better than the best solution so far.
 * Third, the cleanup: Cleaning up all resources which were allocated during the
 * initializtion.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

static volatile sig_atomic_t quit = 0; /** < interrupts main loop */

/**
 * @brief Signal handler for SIGINT and SIGTERM.
 * @details Sets the quit flag to 1 to interrupt the main loop.
 * @param sig Signal number, given by sigaction.
 */
static void onSignal(int signal) {
  fprintf(stderr, "Shutting down - Received user signal: %d\n", signal);
  quit = 1;
}

/**
 * @brief Main loop consisting of 3 segments: Initialization, Main Loop,
 * Cleanup. In the Initialization segment, the shared memory and semaphores are
 * opened and mapped to the process. In the Main Loop segment, the manager
 * receives solutions from the workers and logs them if they are better than the
 * best solution so far. In the Cleanup segment, all resources which were
 * allocated during the Initialization segment are cleaned up.
 *
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int main(int argc, char **argv) {
  if (argc > 1) {
    argumentError("No arguments allowed");
  }

  // register signal handler
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = onSignal;
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    error("sigaction failed");
  }
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    error("sigaction failed");
  }

  // create shared memory -> shm_open()
  int shm_fd = shm_open(SHM_PATH, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    error("shm_open failed");
  }

  // set shared memory size -> ftruncate()
  if (ftruncate(shm_fd, sizeof(CircularBuffer)) == -1) {
    error("ftruncate failed");
  }

  // map shared memory into memory -> mmap()
  CircularBuffer *shm = mmap(NULL, sizeof(CircularBuffer),
                             PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm == MAP_FAILED) {
    error("mmap failed");
  }

  // initialize shared memory
  shm->writeIndex = 0;
  shm->readIndex = 0;
  shm->terminate = false;
  shm->numGenerators = 0;

  // create semaphores -> sem_open()
  sem_t *sem_used_space =
      sem_open(SEM_USED_SPACE_PATH, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
  if (sem_used_space == SEM_FAILED) {
    error("sem_open failed");
  }
  sem_t *sem_available_space =
      sem_open(SEM_AVAILABLE_SPACE_PATH, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR,
               BUFFER_SIZE);
  if (sem_available_space == SEM_FAILED) {
    error("sem_open failed");
  }
  sem_t *sem_mutex =
      sem_open(SEM_MUTEX_PATH, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
  if (sem_mutex == SEM_FAILED) {
    error("sem_open failed");
  }

  EdgeList bestSolution = {.numEdges = SIZE_MAX, .fst = NULL};
  while (!quit) {
    // alternating mutex: wait for used space -> sem_wait()
    if ((sem_wait(sem_used_space) == -1) && (errno != EINTR)) {
      error("sem_wait failed");
    }
    if (quit) {
      break;
    }

    // check solution
    EdgeList submission = shm->buffer[shm->readIndex];
    shm->readIndex = (shm->readIndex + 1) % BUFFER_SIZE;
    if (submission.numEdges == 0) {
      fprintf(stdout, "%s\n", "Input graph is acyclic - Terminating...");
      break;
    } else if (submission.numEdges < bestSolution.numEdges) {
      bestSolution = submission;
      fprintf(stdout, "New best solution: %zu edges\n", bestSolution.numEdges);
      // https://stackoverflow.com/questions/40200227/store-an-string-on-a-shared-memory-c
    }

    // alternating mutex: signal available space -> sem_post()
    if (sem_post(sem_available_space) == -1) {
      error("sem_post failed");
    }
  }

  // wake up all blocked generators, terminate them
  shm->terminate = true;
  log("Waking up all %d generators for termination...\n", shm->numGenerators);
  for (uint64_t i = 0; i < shm->numGenerators; i++) {
    if (sem_post(sem_available_space) == -1) {
      error("sem_post failed");
    }
  }

  // unmap shared memory -> munmap()
  if (munmap(shm, sizeof(CircularBuffer)) == -1) {
    error("munmap failed");
  }

  // close shared memory -> close()
  if (close(shm_fd) == -1) {
    error("close failed");
  }

  // delete shared memory -> shm_unlink()
  if (shm_unlink(SHM_PATH) == -1) {
    error("shm_unlink failed");
  }

  // close semaphores -> sem_close()
  if (sem_close(sem_used_space) == -1) {
    error("sem_close failed");
  }
  if (sem_close(sem_available_space) == -1) {
    error("sem_close failed");
  }
  if (sem_close(sem_mutex) == -1) {
    error("sem_close failed");
  }

  // delete semaphores -> sem_unlink()
  if (sem_unlink(SEM_USED_SPACE_PATH) == -1) {
    error("sem_unlink failed");
  }
  if (sem_unlink(SEM_AVAILABLE_SPACE_PATH) == -1) {
    error("sem_unlink failed");
  }
  if (sem_unlink(SEM_MUTEX_PATH) == -1) {
    error("sem_unlink failed");
  }

  return EXIT_SUCCESS;
}