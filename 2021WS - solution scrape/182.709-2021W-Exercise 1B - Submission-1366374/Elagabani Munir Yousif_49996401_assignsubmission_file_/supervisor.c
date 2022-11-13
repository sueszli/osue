/**
 * @file supervisor.c
 * @author Munir Yousif Elagabani <e12022518@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief program that reads from the buffer and keeps track of the best
 *solution
 **/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "circularbuffer.h"

volatile sig_atomic_t quit = 0;

/**
 * @brief function that handles both SIGINT and SIGTERM signals
 *
 * @param signal
 */
void handle_signal(int signal) { quit = 1; }

/**
 * @brief Main entry point for the progam.
 *
 * @param argc the number of arguments
 * @param argv the array of arguments
 * @return Returns exit code EXIT_SUCCESS OR EXIT_FAILURE
 */

int main(int argc, char **argv) {
  struct sigaction sa = {.sa_handler = handle_signal};
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  char *progname = argv[0];

  if (load_buffer(true, progname)) {
    terminate();
    return EXIT_SUCCESS;
  }

  while (!quit) {
    if (get_min_solution_size() == 0) {
      printf("This graph is already acyclic.");
      quit = 1;
      break;
    }

    fbArc_t solution = read_buffer();

    // we found a better solution
    if (get_min_solution_size() > solution.numEdges) {
      printf("%zu egdes: ", solution.numEdges);
      for (int i = 0; i < solution.numEdges; i++) {
        printf("%d-%d ", solution.selection[i].from, solution.selection[i].to);
      }
      printf("\n");
      set_min_solution_size(solution.numEdges);
    }
  }
  // set variable in buffer to stop all generators
  terminate();

  cleanup_buffer();
  return EXIT_SUCCESS;
}