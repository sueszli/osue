/**
 * @file generator.c
 * @author Jannis Adamek (11809490)
 * @date 2021-11-14
 *
 * @brief Defines the producer of the consumer-producer pattern.
 **/

#include "graph.h"
#include "sharedspace.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

/** Global graph, the generator only ever needs one graph. */
static Graph *graph = NULL;

/** Name of the executable. */
static char *program_name;
/** Strcut that represents the shared space for interprocess comunication. */
static SharedSpace *shared_space = NULL;
/** Semaphores used. */
static sem_t *sem_full = NULL;
static sem_t *sem_free = NULL;
static sem_t *sem_mutex = NULL;
static int shared_space_fd = -1;

static void display_usage_and_exit(void) {
  fprintf(stderr, "Usage: %s EDGE1...\n", program_name);
  exit(EXIT_FAILURE);
}

static void print_error_message(const char *reason) {
  fprintf(stderr, "[%s] ERROR %s: %s\n", program_name, reason, strerror(errno));
}

static void generate_graph_from_args(int argc, char **argv) {
  int number_edges = argc - 1;

  // At least one edge (= positional argument) is required.
  if (number_edges < 1) {
    fprintf(stderr, "[%s] ERROR please provide at least one edge.\n", argv[0]);
    display_usage_and_exit();
  }

  program_name = argv[0];

  graph = malloc_graph(number_edges);

  for (int i = 1; i <= number_edges; i++) {
    add_edge_to_graph(graph, argv[i]);
  }

  malloc_edge_color_arr(graph);
}

static void prepare_shared_space(void) {
  shared_space_fd = shm_open(SHM_NAME, O_RDWR, 0600);
  if (shared_space_fd == -1) {
    print_error_message("shm_open failed and reported");
    exit(EXIT_FAILURE);
  }
  shared_space = mmap(NULL, sizeof(*shared_space), PROT_WRITE | PROT_READ,
                      MAP_SHARED, shared_space_fd, 0);
  if (shared_space == MAP_FAILED) {
    print_error_message("mmap failed and reported");
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief Open the three semaphores that supervisor provides for interprocess
 * communication.
 */
static void init_semaphores(void) {
  sem_full = sem_open(SEM_FULL, 0);
  sem_free = sem_open(SEM_FREE, 0);
  sem_mutex = sem_open(SEM_MUTEX, 0);
}

/**
 * Free all system resources
 * @brief Close the semaphores and the memory map.
 */
void free_resources(void) {
  sem_close(sem_full);
  sem_close(sem_free);
  sem_close(sem_mutex);
  munmap(shared_space, sizeof(*shared_space));
  close(shared_space_fd);
}

/**
 * Prodcue on result and put it onto the circular buffer
 * @brief This is the main loop from the producers. The global graph gets a
 * random coloring and the set of edges, that need to be removed to match a
 * 3-color, is reported to the supervisor process.
 */
static void produce_results() {
  while (shared_space->running) {
    sem_wait(sem_mutex);
    color_graph_randomly(graph);
    sem_wait(sem_free);
    get_excess_edges(graph, &(shared_space->buffer[shared_space->wr_pos]));
    sem_post(sem_full);
    shared_space->wr_pos++;
    shared_space->wr_pos %= CIRCULAR_BUFFER_SIZE;
    sem_post(sem_mutex);
  }
}

/**
 * Main entry point into the program.
 * @param argc Argument count.
 * @param argv Vector of arguments.
 * @returns EXIT_SUCCESS if a perfect solution was found or the user terminated
 * the program, EXIT_FAILURE if memory/semaphore management fails.
 */
int main(int argc, char **argv) {

  generate_graph_from_args(argc, argv);
  prepare_shared_space();
  init_semaphores();

  produce_results();

  free_resources();

  free_graph(graph);

  return EXIT_SUCCESS;
}