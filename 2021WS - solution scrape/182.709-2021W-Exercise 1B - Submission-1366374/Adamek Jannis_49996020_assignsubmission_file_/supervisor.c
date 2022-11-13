/**
 * @file supervisor.c
 * @author Jannis Adamek (11809490)
 * @date 2021-11-14
 *
 * @brief Supervisor program from assignment 1B.
 *
 * Demonstration of the consumer-producer pattern on the example of 3-colorable
 * graphs.
 **/

#include "sharedspace.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

/** Name of the executable */
static char *program_name;
/** Access to the share memory region. */
static SharedSpace *shared_space = NULL;
/** File descriptor from the shared_space, needed for POSIX open/close.. APIs */
static int shared_space_fd = -1;

/** Semaphore that indicates how many items can be taken away at a time. */
static sem_t *sem_full = NULL;
/** Semaphore that indicates how many items can be added to the buffer. */
static sem_t *sem_free = NULL;
/** Semaphore for mutual exclusion when using multiple generators. */
static sem_t *sem_mutex = NULL;
/** Best result that the generator has seen locally. */
static int best_seen_solution = INT_MAX;

/** Global flag that checks whether the program is running, this can be set
 * false by the signal_handler. */
static volatile bool vol_running = true;

/**
 * Handle command line args
 * @brief Assures that there are no arguments provided to the program.
 * @details Sets the global program_name variable.
 * @param argc Argument count as provided to main.
 * @param argv Argument vector as provided to main.
 */
static void handle_args(int argc, char **argv) {
  if (argc != 1) {
    fprintf(stderr,
            "[%s] ERROR program doesn't accept any arguments.\nUsage: %s\n",
            argv[0], argv[0]);
    exit(EXIT_FAILURE);
  }
  program_name = argv[0];
}

/**
 * Handle interrupt signals
 * @brief Sets the vol_running flag to false when interrupted, which ends the
 * main loop in read_from_buffer.
 * @details All generator programs terminate as well, because the is_running
 * flag in the shared memory gets disabled as well.
 * @param signal The integer code of the signal.
 */
void handle_signal(int signal) {
  printf("[%s] Interrupted\n", program_name);
  vol_running = false;
}

/**
 * Close all open resources.
 * @brief Close and unlink the shared memory region and all semaphores.
 */
static void free_resources(void) {
  sem_close(sem_full);
  sem_close(sem_free);
  sem_close(sem_mutex);
  sem_unlink(SEM_FULL);
  sem_unlink(SEM_FREE);
  sem_unlink(SEM_MUTEX);
  munmap(shared_space, sizeof(*shared_space));
  close(shared_space_fd);
  shm_unlink(SHM_NAME);
}

/**
 * Print consistent error messages using errno.
 * @brief Helper function for printing out consitent error messages.
 */
static void print_error_message(const char *reason) {
  fprintf(stderr, "[%s] ERROR %s: %s\n", program_name, reason, strerror(errno));
}

/**
 * Prepare shared memory region for the generators.
 * @brief Uses the typical shm_open - ftruncate - mmap sequence to prepare the
 * global variable shared_space for shared memory usage.
 */
static void prepare_shared_space(void) {
  shared_space_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
  if (shared_space_fd == -1) {
    print_error_message("shm_open reported");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shared_space_fd, sizeof(SharedSpace)) < 0) {
    print_error_message("ftruncate reported");
    exit(EXIT_FAILURE);
  }
  shared_space = mmap(NULL, sizeof(*shared_space), PROT_READ | PROT_WRITE,
                      MAP_SHARED, shared_space_fd, 0);
  if (shared_space == MAP_FAILED) {
    print_error_message("mmap failed and reported");
    exit(EXIT_FAILURE);
  }

  shared_space->rd_pos = 0;
  shared_space->wr_pos = 0;
  shared_space->running = true;

  if (close(shared_space_fd) == -1) {
    print_error_message("tried to close file pointer but close reported");
  }
}

/**
 * Initialize all semaphores.
 * @brief Initialize sem_full, sem_free abd sem_mutex.
 * @details Since the supervisor runs in a 1 to n relationsship with the
 * generators, it is responsible for opening and closing all semaphores.
 * sem_full starts with 0 since removing elements from the circular buffer
 * should only start after the first element has been added. sem_mutex can only
 * ever be 0 or 1, because access to the buffer is restriced to one generator at
 * a time.
 */
void init_semaphores(void) {
  sem_full = sem_open(SEM_FULL, O_CREAT | O_EXCL, 0600, 0);
  if (sem_full == SEM_FAILED) {
    print_error_message("sem_open returned SEM_FAILED and reported");
    sem_full = NULL;
  }
  sem_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, CIRCULAR_BUFFER_SIZE);
  if (sem_free == SEM_FAILED) {
    print_error_message("sem_open returned SEM_FAILED and reported");
    sem_free = NULL;
  }
  sem_mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0600, 1);
  if (sem_mutex == SEM_FAILED) {
    print_error_message("sem_open returned SEM_FAILED and reported");
    sem_mutex = NULL;
  }
}

/**
 * Read from the circular buffer in an infinit loop
 * @brief The main loop of the supervisor, consider one solution from the buffer
 * at a time, if it was better the best_seen_solution it will be printed.
 * @details This is the consumer from the consumer-producer pattern. Because the
 * generators use an approximation algorithm guided by randomness, the program
 * never actually terminates (except when a perfect coloring is found).
 * Solutions get printed onto stdout until the user sends a signal to the
 * process to terminate it.
 */
static void read_from_buffer(void) {
  while (vol_running) {
    sem_wait(sem_full);
    int cur_sol = shared_space->buffer[shared_space->rd_pos].next_index;
    if (cur_sol < best_seen_solution) {
      best_seen_solution = cur_sol;
      printf("[%s] Solution with %d edges: ", program_name, best_seen_solution);
      if (best_seen_solution == 0) {
        printf("The graph is 3-colorable!\n");
        vol_running = false;
      } else {
        print_edge_set(&(shared_space->buffer[shared_space->rd_pos]));
        printf("\n");
      }
    }
    sem_post(sem_free);
    shared_space->rd_pos++;
    shared_space->rd_pos %= CIRCULAR_BUFFER_SIZE;
  }
  shared_space->running = false;
}

/**
 * Main entry point into the program.
 * @param argc Argument count.
 * @param argv Vector of arguments.
 * @returns EXIT_SUCCESS if a perfect solution was found or the user terminated
 * the program, EXIT_FAILURE if any resource management tasks failed.
 */
int main(int argc, char **argv) {
  handle_args(argc, argv);
  prepare_shared_space();
  init_semaphores();

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handle_signal;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  read_from_buffer();
  free_resources();

  return EXIT_SUCCESS;
}